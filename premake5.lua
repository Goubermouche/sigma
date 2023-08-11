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
        path.join(llvm_root, "Release/lib")
    }

    -- determine the file extensions for library links based on the platform
    local lib_ext = os.target() == "windows" and ".lib" or ""
    
    links {
        "LLVMAArch64AsmParser" .. lib_ext,
        "LLVMAArch64CodeGen" .. lib_ext,
        "LLVMAArch64Desc" .. lib_ext,
        "LLVMAArch64Disassembler" .. lib_ext,
        "LLVMAArch64Info" .. lib_ext,
        "LLVMAArch64Utils" .. lib_ext,
        "LLVMAMDGPUAsmParser" .. lib_ext,
        "LLVMAMDGPUCodeGen" .. lib_ext,
        "LLVMAMDGPUDesc" .. lib_ext,
        "LLVMAMDGPUDisassembler" .. lib_ext,
        "LLVMAMDGPUInfo" .. lib_ext,
        "LLVMAMDGPUTargetMCA" .. lib_ext,
        "LLVMAMDGPUUtils" .. lib_ext,
        "LLVMARMAsmParser" .. lib_ext,
        "LLVMARMCodeGen" .. lib_ext,
        "LLVMARMDesc" .. lib_ext,
        "LLVMARMDisassembler" .. lib_ext,
        "LLVMARMInfo" .. lib_ext,
        "LLVMARMUtils" .. lib_ext,
        "LLVMAVRAsmParser" .. lib_ext,
        "LLVMAVRCodeGen" .. lib_ext,
        "LLVMAVRDesc" .. lib_ext,
        "LLVMAVRDisassembler" .. lib_ext,
        "LLVMAVRInfo" .. lib_ext,
        "LLVMAggressiveInstCombine" .. lib_ext,
        "LLVMAnalysis" .. lib_ext,
        "LLVMAsmParser" .. lib_ext,
        "LLVMAsmPrinter" .. lib_ext,
        "LLVMBPFAsmParser" .. lib_ext,
        "LLVMBPFCodeGen" .. lib_ext,
        "LLVMBPFDesc" .. lib_ext,
        "LLVMBPFDisassembler" .. lib_ext,
        "LLVMBPFInfo" .. lib_ext,
        "LLVMBinaryFormat" .. lib_ext,
        "LLVMBitReader" .. lib_ext,
        "LLVMBitWriter" .. lib_ext,
        "LLVMBitstreamReader" .. lib_ext,
        "LLVMCFGuard" .. lib_ext,
        "LLVMCodeGen" .. lib_ext,
        "LLVMCore" .. lib_ext,
        "LLVMCoroutines" .. lib_ext,
        "LLVMCoverage" .. lib_ext,
        "LLVMDWARFLinker" .. lib_ext,
        "LLVMDWARFLinkerParallel" .. lib_ext,
        "LLVMDWP" .. lib_ext,
        "LLVMDebugInfoCodeView" .. lib_ext,
        "LLVMDebugInfoDWARF" .. lib_ext,
        "LLVMDebugInfoGSYM" .. lib_ext,
        "LLVMDebugInfoLogicalView" .. lib_ext,
        "LLVMDebugInfoMSF" .. lib_ext,
        "LLVMDebugInfoPDB" .. lib_ext,
        "LLVMDemangle" .. lib_ext,
        "LLVMDlltoolDriver" .. lib_ext,
        "LLVMExecutionEngine" .. lib_ext,
        "LLVMExtensions" .. lib_ext,
        "LLVMFileCheck" .. lib_ext,
        "LLVMFrontendHLSL" .. lib_ext,
        "LLVMFrontendOpenACC" .. lib_ext,
        "LLVMFrontendOpenMP" .. lib_ext,
        "LLVMFuzzMutate" .. lib_ext,
        "LLVMFuzzerCLI" .. lib_ext,
        "LLVMGlobalISel" .. lib_ext,
        "LLVMHexagonAsmParser" .. lib_ext,
        "LLVMHexagonCodeGen" .. lib_ext,
        "LLVMHexagonDesc" .. lib_ext,
        "LLVMHexagonDisassembler" .. lib_ext,
        "LLVMHexagonInfo" .. lib_ext,
        "LLVMIRPrinter" .. lib_ext,
        "LLVMIRReader" .. lib_ext,
        "LLVMInstCombine" .. lib_ext,
        "LLVMInstrumentation" .. lib_ext,
        "LLVMInterfaceStub" .. lib_ext,
        "LLVMInterpreter" .. lib_ext,
        "LLVMJITLink" .. lib_ext,
        "LLVMLTO" .. lib_ext,
        "LLVMLanaiAsmParser" .. lib_ext,
        "LLVMLanaiCodeGen" .. lib_ext,
        "LLVMLanaiDesc" .. lib_ext,
        "LLVMLanaiDisassembler" .. lib_ext,
        "LLVMLanaiInfo" .. lib_ext,
        "LLVMLibDriver" .. lib_ext,
        "LLVMLineEditor" .. lib_ext,
        "LLVMLinker" .. lib_ext,
        "LLVMLoongArchAsmParser" .. lib_ext,
        "LLVMLoongArchCodeGen" .. lib_ext,
        "LLVMLoongArchDesc" .. lib_ext,
        "LLVMLoongArchDisassembler" .. lib_ext,
        "LLVMLoongArchInfo" .. lib_ext,
        "LLVMMC" .. lib_ext,
        "LLVMMCA" .. lib_ext,
        "LLVMMCDisassembler" .. lib_ext,
        "LLVMMCJIT" .. lib_ext,
        "LLVMMCParser" .. lib_ext,
        "LLVMMIRParser" .. lib_ext,
        "LLVMMSP430AsmParser" .. lib_ext,
        "LLVMMSP430CodeGen" .. lib_ext,
        "LLVMMSP430Desc" .. lib_ext,
        "LLVMMSP430Disassembler" .. lib_ext,
        "LLVMMSP430Info" .. lib_ext,
        "LLVMMipsAsmParser" .. lib_ext,
        "LLVMMipsCodeGen" .. lib_ext,
        "LLVMMipsDesc" .. lib_ext,
        "LLVMMipsDisassembler" .. lib_ext,
        "LLVMMipsInfo" .. lib_ext,
        "LLVMNVPTXCodeGen" .. lib_ext,
        "LLVMNVPTXDesc" .. lib_ext,
        "LLVMNVPTXInfo" .. lib_ext,
        "LLVMObjCARCOpts" .. lib_ext,
        "LLVMObjCopy" .. lib_ext,
        "LLVMObject" .. lib_ext,
        "LLVMObjectYAML" .. lib_ext,
        "LLVMOption" .. lib_ext,
        "LLVMOrcJIT" .. lib_ext,
        "LLVMOrcShared" .. lib_ext,
        "LLVMOrcTargetProcess" .. lib_ext,
        "LLVMPasses" .. lib_ext,
        "LLVMPowerPCAsmParser" .. lib_ext,
        "LLVMPowerPCCodeGen" .. lib_ext,
        "LLVMPowerPCDesc" .. lib_ext,
        "LLVMPowerPCDisassembler" .. lib_ext,
        "LLVMPowerPCInfo" .. lib_ext,
        "LLVMProfileData" .. lib_ext,
        "LLVMRISCVAsmParser" .. lib_ext,
        "LLVMRISCVCodeGen" .. lib_ext,
        "LLVMRISCVDesc" .. lib_ext,
        "LLVMRISCVDisassembler" .. lib_ext,
        "LLVMRISCVInfo" .. lib_ext,
        "LLVMRISCVTargetMCA" .. lib_ext,
        "LLVMRemarks" .. lib_ext,
        "LLVMRuntimeDyld" .. lib_ext,
        "LLVMScalarOpts" .. lib_ext,
        "LLVMSelectionDAG" .. lib_ext,
        "LLVMSparcAsmParser" .. lib_ext,
        "LLVMSparcCodeGen" .. lib_ext,
        "LLVMSparcDesc" .. lib_ext,
        "LLVMSparcDisassembler" .. lib_ext,
        "LLVMSparcInfo" .. lib_ext,
        "LLVMSupport" .. lib_ext,
        "LLVMSymbolize" .. lib_ext,
        "LLVMSystemZAsmParser" .. lib_ext,
        "LLVMSystemZCodeGen" .. lib_ext,
        "LLVMSystemZDesc" .. lib_ext,
        "LLVMSystemZDisassembler" .. lib_ext,
        "LLVMSystemZInfo" .. lib_ext,
        "LLVMTableGen" .. lib_ext,
        "LLVMTarget" .. lib_ext,
        "LLVMTargetParser" .. lib_ext,
        "LLVMTextAPI" .. lib_ext,
        "LLVMTransformUtils" .. lib_ext,
        "LLVMVEAsmParser" .. lib_ext,
        "LLVMVECodeGen" .. lib_ext,
        "LLVMVEDesc" .. lib_ext,
        "LLVMVEDisassembler" .. lib_ext,
        "LLVMVEInfo" .. lib_ext,
        "LLVMVectorize" .. lib_ext,
        "LLVMWebAssemblyAsmParser" .. lib_ext,
        "LLVMWebAssemblyCodeGen" .. lib_ext,
        "LLVMWebAssemblyDesc" .. lib_ext,
        "LLVMWebAssemblyDisassembler" .. lib_ext,
        "LLVMWebAssemblyInfo" .. lib_ext,
        "LLVMWebAssemblyUtils" .. lib_ext,
        "LLVMWindowsDriver" .. lib_ext,
        "LLVMWindowsManifest" .. lib_ext,
        "LLVMX86AsmParser" .. lib_ext,
        "LLVMX86CodeGen" .. lib_ext,
        "LLVMX86Desc" .. lib_ext,
        "LLVMX86Disassembler" .. lib_ext,
        "LLVMX86Info" .. lib_ext,
        "LLVMX86TargetMCA" .. lib_ext,
        "LLVMXCoreCodeGen" .. lib_ext,
        "LLVMXCoreDesc" .. lib_ext,
        "LLVMXCoreDisassembler" .. lib_ext,
        "LLVMXCoreInfo" .. lib_ext,
        "LLVMXRay" .. lib_ext,
        "LLVMipo" .. lib_ext,
        "clangAPINotes" .. lib_ext,
        "clangARCMigrate" .. lib_ext,
        "clangAST" .. lib_ext,
        "clangASTMatchers" .. lib_ext,
        "clangAnalysis" .. lib_ext,
        "clangAnalysisFlowSensitive" .. lib_ext,
        "clangAnalysisFlowSensitiveModels" .. lib_ext,
        "clangBasic" .. lib_ext,
        "clangCodeGen" .. lib_ext,
        "clangCrossTU" .. lib_ext,
        "clangDependencyScanning" .. lib_ext,
        "clangDirectoryWatcher" .. lib_ext,
        "clangDriver" .. lib_ext,
        "clangDynamicASTMatchers" .. lib_ext,
        "clangEdit" .. lib_ext,
        "clangExtractAPI" .. lib_ext,
        "clangFormat" .. lib_ext,
        "clangFrontend" .. lib_ext,
        "clangFrontendTool" .. lib_ext,
        "clangHandleCXX" .. lib_ext,
        "clangHandleLLVM" .. lib_ext,
        "clangIndex" .. lib_ext,
        "clangIndexSerialization" .. lib_ext,
        "clangInterpreter" .. lib_ext,
        "clangLex" .. lib_ext,
        "clangParse" .. lib_ext,
        "clangRewrite" .. lib_ext,
        "clangRewriteFrontend" .. lib_ext,
        "clangSema" .. lib_ext,
        "clangSerialization" .. lib_ext,
        "clangStaticAnalyzerCheckers" .. lib_ext,
        "clangStaticAnalyzerCore" .. lib_ext,
        "clangStaticAnalyzerFrontend" .. lib_ext,
        "clangSupport" .. lib_ext,
        "clangTooling" .. lib_ext,
        "clangToolingASTDiff" .. lib_ext,
        "clangToolingCore" .. lib_ext,
        "clangToolingInclusions" .. lib_ext,
        "clangToolingInclusionsStdlib" .. lib_ext,
        "clangToolingRefactoring" .. lib_ext,
        "clangToolingSyntax" .. lib_ext,
        "clangTransformer" .. lib_ext
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