NAME = math-engine

# ── OS detection ────────────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
  PLATFORM   = windows
  CXX        = clang++
  BIN        = $(NAME).exe
  MKDIR      = if not exist $(OBJS_DIR) mkdir $(OBJS_DIR)
  RM         = if exist $(OBJS_DIR) rmdir /s /q $(OBJS_DIR)
  RM_FILE    = if exist $(BIN) del /q $(BIN)
  RM_OUTPUT  = if exist output rmdir /s /q output
  RUN_CMD    = $(BIN)
  NULL_DEV   = NUL
else
  PLATFORM   = linux
  CXX        = c++
  BIN        = $(NAME)
  MKDIR      = mkdir -p $(OBJS_DIR)
  RM         = rm -rf $(OBJS_DIR)
  RM_FILE    = rm -f $(BIN)
  RM_OUTPUT  = rm -rf output
  RUN_CMD    = ./$(BIN)
  NULL_DEV   = /dev/null
endif

# ── Compiler flags ───────────────────────────────────────────────────
CXXFLAGS_BASE = -Wall -Wextra -Werror -std=c++17
INCLUDES      = -I includes

# ── Platform-specific flags ──────────────────────────────────────────
ifeq ($(PLATFORM),windows)

  # ----------------------------------------------------------------
  # WebView2 SDK — extract the NuGet package into libs/webview2/
  #   Headers : libs/webview2/include/
  #   Libs    : libs/webview2/x64/
  #
  # zstd — place static lib in libs/zstd/  (or install via vcpkg)
  #   Headers : libs/zstd/include/
  #   Lib     : libs/zstd/lib/zstd.lib
  # ----------------------------------------------------------------

  WEBVIEW2_DIR = libs/webview2
  ZSTD_DIR     = libs/zstd

  CXXFLAGS_PLATFORM = \
    --target=x86_64-pc-windows-msvc \
    -I $(WEBVIEW2_DIR)/include \
    -I $(ZSTD_DIR)/include \
    -DWEBVIEW_EDGE \
    -DUNICODE -D_UNICODE \
    -D_CRT_SECURE_NO_WARNINGS

  # clang_rt.builtins provides ___chkstk_ms used by MinGW-built zstd
  CLANG_RT_DIR  = $(shell clang++ --print-runtime-dir 2>$(NULL_DEV))

  LIBS_PLATFORM = \
    --target=x86_64-pc-windows-msvc \
    -L $(WEBVIEW2_DIR)/x64 \
    -L $(ZSTD_DIR)/lib \
    -L "$(CLANG_RT_DIR)" \
    -lWebView2Loader \
    -lzstd \
    -lclang_rt.builtins-x86_64 \
    -lole32 -loleaut32 -lshlwapi \
    -luser32 -lgdi32 -lkernel32 \
    -mwindows

else

  # ----------------------------------------------------------------
  # Linux — gtk+-3.0 + webkit2gtk-4.1 (or 4.0 fallback) + zstd
  # ----------------------------------------------------------------

  PKG_CXXFLAGS = $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.1 2>$(NULL_DEV) \
                      || pkg-config --cflags gtk+-3.0 webkit2gtk-4.0 2>$(NULL_DEV) \
                      || echo "")

  PKG_LIBS     = $(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.1 2>$(NULL_DEV) \
                      || pkg-config --libs gtk+-3.0 webkit2gtk-4.0 2>$(NULL_DEV) \
                      || echo "")

  CXXFLAGS_PLATFORM = $(PKG_CXXFLAGS)
  LIBS_PLATFORM     = -lzstd $(PKG_LIBS)

endif

CXXFLAGS = $(CXXFLAGS_BASE) $(CXXFLAGS_PLATFORM) $(INCLUDES)
LIBS     = $(LIBS_PLATFORM)

# ── Sources ──────────────────────────────────────────────────────────
SRCS_DIR = srcs
SRCS     = $(SRCS_DIR)/main.cpp \
           $(SRCS_DIR)/Distribution.cpp \
           $(SRCS_DIR)/ModeManager.cpp \
           $(SRCS_DIR)/AppBridge.cpp

ifeq ($(PLATFORM),windows)
  SRCS += $(SRCS_DIR)/win_stubs.cpp
endif

OBJS_DIR  = objs
OBJS_CPP  = $(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(filter %.cpp,$(SRCS)))
OBJS_C    = $(patsubst $(SRCS_DIR)/%.c,$(OBJS_DIR)/%.o,$(filter %.c,$(SRCS)))
OBJS      = $(OBJS_CPP) $(OBJS_C)

# ── Rules ─────────────────────────────────────────────────────────────
all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) -std=c++17 $(OBJS) -o $(BIN) $(LIBS)
ifeq ($(PLATFORM),windows)
	@copy /Y $(WEBVIEW2_DIR)\x64\WebView2Loader.dll . >$(NULL_DEV)
endif

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM)

fclean: clean
	$(RM_FILE)
	$(RM_OUTPUT)

re: fclean all

run: all
	$(RUN_CMD)

# ── Windows setup helper ──────────────────────────────────────────────
# Run `make setup-win` once to download and extract all Windows dependencies.
# Requires: curl, powershell (both available in any modern Windows terminal)
ifeq ($(PLATFORM),windows)
setup-win:
	@echo [1/3] Creating libs directories...
	@if not exist libs\webview2\include mkdir libs\webview2\include
	@if not exist libs\webview2\x64     mkdir libs\webview2\x64
	@if not exist libs\zstd\include     mkdir libs\zstd\include
	@if not exist libs\zstd\lib         mkdir libs\zstd\lib
	@echo [2/3] Downloading Microsoft.Web.WebView2 NuGet package...
	curl -L "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2" -o libs\webview2.nupkg
	powershell -Command "Expand-Archive -Force libs\webview2.nupkg libs\webview2_pkg"
	xcopy /E /Y libs\webview2_pkg\build\native\include\*   libs\webview2\include\
	xcopy /Y   libs\webview2_pkg\build\native\x64\*        libs\webview2\x64\
	@echo [3/3] Downloading zstd...
	curl -L "https://github.com/facebook/zstd/releases/latest/download/zstd-v1.5.6-win64.zip" -o libs\zstd.zip
	powershell -Command "Expand-Archive -Force libs\zstd.zip libs\zstd_pkg"
	xcopy /Y libs\zstd_pkg\zstd-v*\include\* libs\zstd\include\
	xcopy /Y libs\zstd_pkg\zstd-v*\lib\*     libs\zstd\lib\
	@echo Done. Run `make` to build.
endif

.PHONY: all clean fclean re run setup-win
