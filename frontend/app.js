const { createApp } = Vue;

createApp({
    data() {
        return {
            modes: [],
            sims: 100000,
            selectedModeIdx: 0,
            isRunning: false,
            statusMessage: '',
            statusType: 'info',
            _statusTimer: null,
            _pendingAction: null  // 'load' | 'save' | 'export'
        };
    },

    computed: {
        selectedMode() {
            return this.modes[this.selectedModeIdx] || null;
        },

        rtpGaugeColor() {
            const rtp = this.selectedMode?.rtp || 0;
            if (rtp >= 0.97) return '#30D158';
            if (rtp >= 0.94) return '#0A84FF';
            if (rtp >= 0.88) return '#FF9F0A';
            return '#FF453A';
        },

        volatilityLabel() {
            const v = this.selectedMode?.stats?.volatility;
            if (v == null) return '—';
            if (v < 0.5)  return 'Low';
            if (v < 1.0)  return 'Medium';
            if (v < 2.0)  return 'High';
            return 'Very High';
        },

        // 1–5 segments filled
        volatilityLevel() {
            const v = this.selectedMode?.stats?.volatility;
            if (v == null) return 0;
            if (v < 0.3)  return 1;
            if (v < 0.7)  return 2;
            if (v < 1.2)  return 3;
            if (v < 2.0)  return 4;
            return 5;
        },

        volatilityColor() {
            switch (this.volatilityLevel) {
                case 1: return '#30D158';
                case 2: return '#FFD60A';
                case 3: return '#FF9F0A';
                case 4: return '#FF6B00';
                default: return '#FF453A';
            }
        },

        // Multipliers sorted ascending by value for the distribution chart
        sortedMultipliers() {
            if (!this.selectedMode) return [];
            return [...this.selectedMode.multipliers].sort((a, b) => a.m - b.m);
        }
    },

    mounted() {
        window.addEventListener('backend-response', (e) => {
            this.isRunning = false;
            if (e.detail?.modes) {
                const prevIdx = this.selectedModeIdx;
                this.modes = e.detail.modes;
                this.selectedModeIdx = Math.min(prevIdx, Math.max(0, this.modes.length - 1));
                this.setStatus('Data refreshed', 'success');
            } else if (e.detail?.configPath !== undefined) {
                const path = e.detail.configPath;
                if (path && this._pendingAction === 'load') {
                    this.invoke('load', path);
                } else if (path && this._pendingAction === 'save') {
                    this.invoke('save', path);
                    this.setStatus('Saved', 'success');
                }
                this._pendingAction = null;
            } else if (e.detail?.outputDir !== undefined) {
                const dir = e.detail.outputDir;
                if (dir && this._pendingAction === 'export') {
                    this.invoke('export', dir);
                }
                this._pendingAction = null;
            } else if (e.detail?.status === 'ok') {
                this.setStatus('Done', 'success');
            } else if (e.detail?.error) {
                this.setStatus(e.detail.error, 'error');
            }
        });
        this.refresh();
    },

    methods: {
        invoke(fn, arg) {
            if (window.external?.invoke) {
                window.external.invoke(JSON.stringify({ fn, arg }));
            }
        },

        refresh()   { this.invoke('getModes', ''); },

        runSims() {
            this.isRunning = true;
            this.setStatus(`Running ${this.formatCount(this.sims)} iterations…`, 'info');
            this.invoke('runSims', this.sims.toString());
        },

        updateModeName(idx)  { this.invoke('updateModeName', `${idx}|${this.modes[idx].name}`); },
        updateModeCost(idx)  { this.invoke('updateModeCost', `${idx}|${this.modes[idx].cost}`); },

        updateMultiplier(idx, mIdx) {
            const m = this.modes[idx].multipliers[mIdx];
            this.invoke('updateMultiplier', `${idx}|${mIdx}|${m.m}|${m.w}`);
        },

        updateFreeSpins(idx) {
            const fs = this.modes[idx].freeSpins;
            this.invoke('updateFreeSpins',
                `${idx}|${fs.enabled?1:0}|${fs.triggerWeight}|${fs.count}|${fs.multiplierBoost}|${fs.canRetrigger?1:0}`
            );
        },

        addMode()   { this.invoke('addMode', ''); },

        removeMode(idx) {
            if (confirm('Delete this mode?')) {
                this.invoke('removeMode', idx.toString());
            }
        },

        importWithPicker() {
            this._pendingAction = 'load';
            this.invoke('browseConfig', 'open');
        },
        saveWithPicker() {
            this._pendingAction = 'save';
            this.invoke('browseConfig', 'save');
        },
        exportWithPicker() {
            this._pendingAction = 'export';
            this.invoke('browseOutput', '');
        },

        addMultiplier(idx)             { this.invoke('addMultiplier', idx.toString()); },
        removeMultiplier(idx, mIdx)    { this.invoke('removeMultiplier', `${idx}|${mIdx}`); },

        calculateProb(mode, mIdx) {
            const total = mode.multipliers.reduce((s, m) => s + m.w, 0);
            if (!total) return '0.00';
            return ((mode.multipliers[mIdx].w / total) * 100).toFixed(2);
        },

        // Distribution chart helpers (operate on selectedMode)
        getBarWidth(m) {
            if (!this.selectedMode?.multipliers?.length) return 0;
            const maxW = Math.max(...this.selectedMode.multipliers.map(x => x.w));
            return maxW > 0 ? Math.round((m.w / maxW) * 100) : 0;
        },

        getProb(m) {
            if (!this.selectedMode?.multipliers?.length) return '0.0';
            const total = this.selectedMode.multipliers.reduce((s, x) => s + x.w, 0);
            return total > 0 ? ((m.w / total) * 100).toFixed(1) : '0.0';
        },

        barColor(mult) {
            if (mult === 0)   return '#FF453A'; // loss
            if (mult < 1)     return '#FF9F0A'; // partial
            if (mult < 5)     return '#30D158'; // small win
            if (mult < 20)    return '#0A84FF'; // medium win
            return '#BF5AF2';                   // big win
        },

        formatCount(n) {
            if (n >= 1000000) return (n / 1000000).toFixed(0) + 'M';
            if (n >= 1000)    return (n / 1000).toFixed(0) + 'K';
            return n.toString();
        },

        setStatus(msg, type = 'info') {
            this.statusMessage = msg;
            this.statusType    = type;
            clearTimeout(this._statusTimer);
            if (type !== 'info') {
                this._statusTimer = setTimeout(() => { this.statusMessage = ''; }, 3000);
            }
        }
    }
}).mount('#app');
