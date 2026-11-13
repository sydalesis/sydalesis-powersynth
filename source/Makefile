#!/usr/bin/make -f
NAME = Sydalesis
# Link order matters: the generic engine must come before the AVX2 one (see EngineCore.hpp notes).
FILES_DSP = EngineGeneric.cpp DriftfieldPlugin.cpp EngineFactory.cpp Platform.cpp mts/libMTSClient.cpp EngineAVX2.cpp
FILES_UI  = DriftfieldUI.cpp
DPF_PATH ?= ../dpf
DPF_TARGET_DIR ?= bin
DPF_BUILD_DIR ?= build
include $(DPF_PATH)/Makefile.plugins.mk
BUILD_CXX_FLAGS += -O3 -ffast-math -fno-finite-math-only -std=gnu++17
ifeq ($(MACOS),true)
# 5.1.6 macOS universal build: AVX2/FMA only for the Intel slice (the arm64 slice compiles it as plain code
# and never selects it at run time)
$(BUILD_DIR)/EngineAVX2.cpp.o: BUILD_CXX_FLAGS += -Xarch_x86_64 -mavx2 -Xarch_x86_64 -mfma
else ifneq (,$(filter x86_64 i686 i386 amd64,$(shell $(CXX) -dumpmachine | cut -d- -f1)))
$(BUILD_DIR)/EngineAVX2.cpp.o: BUILD_CXX_FLAGS += -mavx2 -mfma
endif
# 6.0.5: editor frame pacer (Platform.cpp) subclasses the editor window
ifeq ($(WINDOWS),true)
EXTRA_UI_LIBS += -lcomctl32
endif
TARGETS += vst3 clap
ifeq ($(MACOS),true)
TARGETS += au
endif
all: $(TARGETS)
