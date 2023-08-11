-- get the LLVM installation path
local llvm_root = os.getenv("LLVM_ROOT")
local script_root = path.getabsolute(".")
print("LLVM_ROOT: " .. llvm_root)

workspace "sigma"
    configurations { "Release" }
    architecture "x64"
    startproject "compiler"
    
project "compiler"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"

    location "compiler"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")

    externalanglebrackets "On"
    externalwarnings "Off"
    
    flags {
        "MultiProcessorCompile"
    }

    files {
        "compiler/source/**.*",
        "compiler/main.cpp"
    }

    includedirs {
        "compiler/source",
        path.join(llvm_root, "include")
    }

    libdirs {   
        path.join(llvm_root, "lib"),
        path.join(llvm_root, "bin"),
        path.join(llvm_root, "../llvm-build/lib"),
        path.join(llvm_root, "Release/lib")
    }

    links {
        "LLVMAArch64AsmParser",
        "LLVMAArch64CodeGen",
        "LLVMAArch64Desc",
        "LLVMAArch64Disassembler",
        "LLVMAArch64Info",
        "LLVMAArch64Utils",
        "LLVMAMDGPUAsmParser",
        "LLVMAMDGPUCodeGen",
        "LLVMAMDGPUDesc",
        "LLVMAMDGPUDisassembler",
        "LLVMAMDGPUInfo",
        "LLVMAMDGPUTargetMCA",
        "LLVMAMDGPUUtils",
        "LLVMARMAsmParser",
        "LLVMARMCodeGen",
        "LLVMARMDesc",
        "LLVMARMDisassembler",
        "LLVMARMInfo",
        "LLVMARMUtils",
        "LLVMAVRAsmParser",
        "LLVMAVRCodeGen",
        "LLVMAVRDesc",
        "LLVMAVRDisassembler",
        "LLVMAVRInfo",
        "LLVMAggressiveInstCombine",
        "LLVMAnalysis",
        "LLVMAsmParser",
        "LLVMAsmPrinter",
        "LLVMBPFAsmParser",
        "LLVMBPFCodeGen",
        "LLVMBPFDesc",
        "LLVMBPFDisassembler",
        "LLVMBPFInfo",
        "LLVMBinaryFormat",
        "LLVMBitReader",
        "LLVMBitWriter",
        "LLVMBitstreamReader",
        "LLVMCFGuard",
        "LLVMCodeGen",
        "LLVMCore",
        "LLVMCoroutines",
        "LLVMCoverage",
        "LLVMDWARFLinker",
        "LLVMDWARFLinkerParallel",
        "LLVMDWP",
        "LLVMDebugInfoCodeView",
        "LLVMDebugInfoDWARF",
        "LLVMDebugInfoGSYM",
        "LLVMDebugInfoLogicalView",
        "LLVMDebugInfoMSF",
        "LLVMDebugInfoPDB",
        "LLVMDemangle",
        "LLVMDlltoolDriver",
        "LLVMExecutionEngine",
        "LLVMExtensions",
        "LLVMFileCheck",
        "LLVMFrontendHLSL",
        "LLVMFrontendOpenACC",
        "LLVMFrontendOpenMP",
        "LLVMFuzzMutate",
        "LLVMFuzzerCLI",
        "LLVMGlobalISel",
        "LLVMHexagonAsmParser",
        "LLVMHexagonCodeGen",
        "LLVMHexagonDesc",
        "LLVMHexagonDisassembler",
        "LLVMHexagonInfo",
        "LLVMIRPrinter",
        "LLVMIRReader",
        "LLVMInstCombine",
        "LLVMInstrumentation",
        "LLVMInterfaceStub",
        "LLVMInterpreter",
        "LLVMJITLink",
        "LLVMLTO",
        "LLVMLanaiAsmParser",
        "LLVMLanaiCodeGen",
        "LLVMLanaiDesc",
        "LLVMLanaiDisassembler",
        "LLVMLanaiInfo",
        "LLVMLibDriver",
        "LLVMLineEditor",
        "LLVMLinker",
        "LLVMLoongArchAsmParser",
        "LLVMLoongArchCodeGen",
        "LLVMLoongArchDesc",
        "LLVMLoongArchDisassembler",
        "LLVMLoongArchInfo",
        "LLVMMC",
        "LLVMMCA",
        "LLVMMCDisassembler",
        "LLVMMCJIT",
        "LLVMMCParser",
        "LLVMMIRParser",
        "LLVMMSP430AsmParser",
        "LLVMMSP430CodeGen",
        "LLVMMSP430Desc",
        "LLVMMSP430Disassembler",
        "LLVMMSP430Info",
        "LLVMMipsAsmParser",
        "LLVMMipsCodeGen",
        "LLVMMipsDesc",
        "LLVMMipsDisassembler",
        "LLVMMipsInfo",
        "LLVMNVPTXCodeGen",
        "LLVMNVPTXDesc",
        "LLVMNVPTXInfo",
        "LLVMObjCARCOpts",
        "LLVMObjCopy",
        "LLVMObject",
        "LLVMObjectYAML",
        "LLVMOption",
        "LLVMOrcJIT",
        "LLVMOrcShared",
        "LLVMOrcTargetProcess",
        "LLVMPasses",
        "LLVMPowerPCAsmParser",
        "LLVMPowerPCCodeGen",
        "LLVMPowerPCDesc",
        "LLVMPowerPCDisassembler",
        "LLVMPowerPCInfo",
        "LLVMProfileData",
        "LLVMRISCVAsmParser",
        "LLVMRISCVCodeGen",
        "LLVMRISCVDesc",
        "LLVMRISCVDisassembler",
        "LLVMRISCVInfo",
        "LLVMRISCVTargetMCA",
        "LLVMRemarks",
        "LLVMRuntimeDyld",
        "LLVMScalarOpts",
        "LLVMSelectionDAG",
        "LLVMSparcAsmParser",
        "LLVMSparcCodeGen",
        "LLVMSparcDesc",
        "LLVMSparcDisassembler",
        "LLVMSparcInfo",
        "LLVMSupport",
        "LLVMSymbolize",
        "LLVMSystemZAsmParser",
        "LLVMSystemZCodeGen",
        "LLVMSystemZDesc",
        "LLVMSystemZDisassembler",
        "LLVMSystemZInfo",
        "LLVMTableGen",
        "LLVMTarget",
        "LLVMTargetParser",
        "LLVMTextAPI",
        "LLVMTransformUtils",
        "LLVMVEAsmParser",
        "LLVMVECodeGen",
        "LLVMVEDesc",
        "LLVMVEDisassembler",
        "LLVMVEInfo",
        "LLVMVectorize",
        "LLVMWebAssemblyAsmParser",
        "LLVMWebAssemblyCodeGen",
        "LLVMWebAssemblyDesc",
        "LLVMWebAssemblyDisassembler",
        "LLVMWebAssemblyInfo",
        "LLVMWebAssemblyUtils",
        "LLVMWindowsDriver",
        "LLVMWindowsManifest",
        "LLVMX86AsmParser",
        "LLVMX86CodeGen",
        "LLVMX86Desc",
        "LLVMX86Disassembler",
        "LLVMX86Info",
        "LLVMX86TargetMCA",
        "LLVMXCoreCodeGen",
        "LLVMXCoreDesc",
        "LLVMXCoreDisassembler",
        "LLVMXCoreInfo",
        "LLVMXRay",
        "LLVMipo",
        "clangAPINotes",
        "clangARCMigrate",
        "clangAST",
        "clangASTMatchers",
        "clangAnalysis",
        "clangAnalysisFlowSensitive",
        "clangAnalysisFlowSensitiveModels",
        "clangBasic",
        "clangCodeGen",
        "clangCrossTU",
        "clangDependencyScanning",
        "clangDirectoryWatcher",
        "clangDriver",
        "clangDynamicASTMatchers",
        "clangEdit",
        "clangExtractAPI",
        "clangFormat",
        "clangFrontend",
        "clangFrontendTool",
        "clangHandleCXX",
        "clangHandleLLVM",
        "clangIndex",
        "clangIndexSerialization",
        "clangInterpreter",
        "clangLex",
        "clangParse",
        "clangRewrite",
        "clangRewriteFrontend",
        "clangSema",
        "clangSerialization",
        "clangStaticAnalyzerCheckers",
        "clangStaticAnalyzerCore",
        "clangStaticAnalyzerFrontend",
        "clangSupport",
        "clangTooling",
        "clangToolingASTDiff",
        "clangToolingCore",
        "clangToolingInclusions",
        "clangToolingInclusionsStdlib",
        "clangToolingRefactoring",
        "clangToolingSyntax",
        "clangTransformer"
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"

    filter "system:windows"
        links {
            "comdlg32.lib",
            "delayimp.lib",
            "gdi32.lib",
            "kernel32.lib",
            "ole32.dll",
            "ole32.lib",
            "oleaut32.lib",
            "psapi.lib",
            "shell32.dll",
            "shell32.lib",
            "user32.lib",
            "uuid.lib",
            "version.lib",
            "winspool.lib",
            "advapi32.lib"
        }

        
    filter { "system:linux", "action:gmake" }
        links {
            "dl",     
            "pthread",
            "m",      
            "rt"      
        }

        buildoptions { 
            "-std=c++23"
        }

        linkoptions {
            "-lstdc++"
        }

    filter {}
