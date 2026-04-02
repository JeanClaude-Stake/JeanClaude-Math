const { createApp } = Vue;

createApp({
    data() {
        return {
            modes: [],
            sims: 100000,
            configPath: 'config.json',
            outputDir: 'output'
        };
    },
    mounted() {
        window.addEventListener('backend-response', (e) => {
            console.log('[FRONTEND] Received data from backend:', e.detail);
            if (e.detail && e.detail.modes) {
                this.modes = e.detail.modes;
            }
        });
        this.refresh();
    },
    methods: {
        invoke(fn, arg) {
            console.log('[FRONTEND] Calling backend:', fn, arg);
            if (window.external && window.external.invoke) {
                window.external.invoke(JSON.stringify({fn, arg}));
            }
        },
        refresh() { this.invoke('getModes', ''); },
        runSims() { this.invoke('runSims', this.sims.toString()); },
        updateModeName(idx) { this.invoke('updateModeName', `${idx}|${this.modes[idx].name}`); },
        updateModeCost(idx) { this.invoke('updateModeCost', `${idx}|${this.modes[idx].cost}`); },
        updateMultiplier(idx, mIdx) {
            const m = this.modes[idx].multipliers[mIdx];
            this.invoke('updateMultiplier', `${idx}|${mIdx}|${m.m}|${m.w}`);
        },
        updateFreeSpins(idx) {
            const fs = this.modes[idx].freeSpins;
            const arg = `${idx}|${fs.enabled?1:0}|${fs.triggerWeight}|${fs.count}|${fs.multiplierBoost}|${fs.canRetrigger?1:0}`;
            this.invoke('updateFreeSpins', arg);
        },
        addMode() { this.invoke('addMode', ''); },
        removeMode(idx) {
            if (confirm('Are you sure you want to delete this mode?')) {
                this.invoke('removeMode', idx.toString());
            }
        },
        addMultiplier(idx) { this.invoke('addMultiplier', idx.toString()); },
        removeMultiplier(idx, mIdx) { this.invoke('removeMultiplier', `${idx}|${mIdx}`); },
        calculateProb(mode, mIdx) {
            const total = mode.multipliers.reduce((sum, m) => sum + m.w, 0);
            if (total === 0) return 0;
            return ((mode.multipliers[mIdx].w / total) * 100).toFixed(2);
        }
    }
}).mount('#app');
