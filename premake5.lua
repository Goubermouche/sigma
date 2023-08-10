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
    
    flags 
    {
        "MultiProcessorCompile"
    }

    files
    {
        "compiler/source/**.*",
        "compiler/main.cpp"
    }

    includedirs
    {
        "compiler/source",
        path.join(llvm_root, "include")
    }

    libdirs
    {   
        path.join(llvm_root, "lib"),
        path.join(llvm_root, "bin"),
        path.join(llvm_root, "Release/lib")
    }

    -- Determine the file extensions for library links based on the platform
    local ext = os.target() == "windows" and ".lib" or ""

    print(ext)

    links 
    {
        "LLVMAArch64AsmParser" .. ext,
        "LLVMAArch64CodeGen" .. ext,
        "LLVMAArch64Desc" .. ext,
        "LLVMAArch64Disassembler" .. ext,
        "LLVMAArch64Info" .. ext,
        "LLVMAArch64Utils" .. ext,
        "LLVMAMDGPUAsmParser" .. ext,
        "LLVMAMDGPUCodeGen" .. ext,
        "LLVMAMDGPUDesc" .. ext,
        "LLVMAMDGPUDisassembler" .. ext,
        "LLVMAMDGPUInfo" .. ext,
        "LLVMAMDGPUTargetMCA" .. ext,
        "LLVMAMDGPUUtils" .. ext,
        "LLVMARMAsmParser" .. ext,
        "LLVMARMCodeGen" .. ext,
        "LLVMARMDesc" .. ext,
        "LLVMARMDisassembler" .. ext,
        "LLVMARMInfo" .. ext,
        "LLVMARMUtils" .. ext,
        "LLVMAVRAsmParser" .. ext,
        "LLVMAVRCodeGen" .. ext,
        "LLVMAVRDesc" .. ext,
        "LLVMAVRDisassembler" .. ext,
        "LLVMAVRInfo" .. ext,
        "LLVMAggressiveInstCombine" .. ext,
        "LLVMAnalysis" .. ext,
        "LLVMAsmParser" .. ext,
        "LLVMAsmPrinter" .. ext,
        "LLVMBPFAsmParser" .. ext,
        "LLVMBPFCodeGen" .. ext,
        "LLVMBPFDesc" .. ext,
        "LLVMBPFDisassembler" .. ext,
        "LLVMBPFInfo" .. ext,
        "LLVMBinaryFormat" .. ext,
        "LLVMBitReader" .. ext,
        "LLVMBitWriter" .. ext,
        "LLVMBitstreamReader" .. ext,
        "LLVMCFGuard" .. ext,
        -- "LLVMCodeGen" .. ext,
        "LLVMCore" .. ext,
        "LLVMCoroutines" .. ext,
        "LLVMCoverage" .. ext,
        "LLVMDWARFLinker" .. ext,
        "LLVMDWARFLinkerParallel" .. ext,
        "LLVMDWP" .. ext,
        "LLVMDebugInfoCodeView" .. ext,
        "LLVMDebugInfoDWARF" .. ext,
        "LLVMDebugInfoGSYM" .. ext,
        "LLVMDebugInfoLogicalView" .. ext,
        "LLVMDebugInfoMSF" .. ext,
        "LLVMDebugInfoPDB" .. ext,
        "LLVMDemangle" .. ext,
        "LLVMDlltoolDriver" .. ext,
        "LLVMExecutionEngine" .. ext,
        "LLVMExtensions" .. ext,
        "LLVMFileCheck" .. ext,
        "LLVMFrontendHLSL" .. ext,
        "LLVMFrontendOpenACC" .. ext,
        "LLVMFrontendOpenMP" .. ext,
        "LLVMFuzzMutate" .. ext,
        "LLVMFuzzerCLI" .. ext,
        "LLVMGlobalISel" .. ext,
        "LLVMHexagonAsmParser" .. ext,
        "LLVMHexagonCodeGen" .. ext,
        "LLVMHexagonDesc" .. ext,
        "LLVMHexagonDisassembler" .. ext,
        "LLVMHexagonInfo" .. ext,
        "LLVMIRPrinter" .. ext,
        "LLVMIRReader" .. ext,
        "LLVMInstCombine" .. ext,
        "LLVMInstrumentation" .. ext,
        "LLVMInterfaceStub" .. ext,
        "LLVMInterpreter" .. ext,
        "LLVMJITLink" .. ext,
        "LLVMLTO" .. ext,
        "LLVMLanaiAsmParser" .. ext,
        "LLVMLanaiCodeGen" .. ext,
        "LLVMLanaiDesc" .. ext,
        "LLVMLanaiDisassembler" .. ext,
        "LLVMLanaiInfo" .. ext,
        "LLVMLibDriver" .. ext,
        "LLVMLineEditor" .. ext,
        "LLVMLinker" .. ext,
        "LLVMLoongArchAsmParser" .. ext,
        "LLVMLoongArchCodeGen" .. ext,
        "LLVMLoongArchDesc" .. ext,
        "LLVMLoongArchDisassembler" .. ext,
        "LLVMLoongArchInfo" .. ext,
        "LLVMMC" .. ext,
        "LLVMMCA" .. ext,
        "LLVMMCDisassembler" .. ext,
        "LLVMMCJIT" .. ext,
        "LLVMMCParser" .. ext,
        "LLVMMIRParser" .. ext,
        "LLVMMSP430AsmParser" .. ext,
        "LLVMMSP430CodeGen" .. ext,
        "LLVMMSP430Desc" .. ext,
        "LLVMMSP430Disassembler" .. ext,
        "LLVMMSP430Info" .. ext,
        "LLVMMipsAsmParser" .. ext,
        "LLVMMipsCodeGen" .. ext,
        "LLVMMipsDesc" .. ext,
        "LLVMMipsDisassembler" .. ext,
        "LLVMMipsInfo" .. ext,
        "LLVMNVPTXCodeGen" .. ext,
        "LLVMNVPTXDesc" .. ext,
        "LLVMNVPTXInfo" .. ext,
        "LLVMObjCARCOpts" .. ext,
        "LLVMObjCopy" .. ext,
        "LLVMObject" .. ext,
        "LLVMObjectYAML" .. ext,
        "LLVMOption" .. ext,
        "LLVMOrcJIT" .. ext,
        "LLVMOrcShared" .. ext,
        "LLVMOrcTargetProcess" .. ext,
        "LLVMPasses" .. ext,
        "LLVMPowerPCAsmParser" .. ext,
        "LLVMPowerPCCodeGen" .. ext,
        "LLVMPowerPCDesc" .. ext,
        "LLVMPowerPCDisassembler" .. ext,
        "LLVMPowerPCInfo" .. ext,
        "LLVMProfileData" .. ext,
        "LLVMRISCVAsmParser" .. ext,
        "LLVMRISCVCodeGen" .. ext,
        "LLVMRISCVDesc" .. ext,
        "LLVMRISCVDisassembler" .. ext,
        "LLVMRISCVInfo" .. ext,
        "LLVMRISCVTargetMCA" .. ext,
        "LLVMRemarks" .. ext,
        "LLVMRuntimeDyld" .. ext,
        "LLVMScalarOpts" .. ext,
        "LLVMSelectionDAG" .. ext,
        "LLVMSparcAsmParser" .. ext,
        "LLVMSparcCodeGen" .. ext,
        "LLVMSparcDesc" .. ext,
        "LLVMSparcDisassembler" .. ext,
        "LLVMSparcInfo" .. ext,
        "LLVMSupport" .. ext,
        "LLVMSymbolize" .. ext,
        "LLVMSystemZAsmParser" .. ext,
        "LLVMSystemZCodeGen" .. ext,
        "LLVMSystemZDesc" .. ext,
        "LLVMSystemZDisassembler" .. ext,
        "LLVMSystemZInfo" .. ext,
        "LLVMTableGen" .. ext,
        "LLVMTarget" .. ext,
        "LLVMTargetParser" .. ext,
        "LLVMTextAPI" .. ext,
        "LLVMTransformUtils" .. ext,
        "LLVMVEAsmParser" .. ext,
        "LLVMVECodeGen" .. ext,
        "LLVMVEDesc" .. ext,
        "LLVMVEDisassembler" .. ext,
        "LLVMVEInfo" .. ext,
        "LLVMVectorize" .. ext,
        "LLVMWebAssemblyAsmParser" .. ext,
        "LLVMWebAssemblyCodeGen" .. ext,
        "LLVMWebAssemblyDesc" .. ext,
        "LLVMWebAssemblyDisassembler" .. ext,
        "LLVMWebAssemblyInfo" .. ext,
        "LLVMWebAssemblyUtils" .. ext,
        "LLVMWindowsDriver" .. ext,
        "LLVMWindowsManifest" .. ext,
        "LLVMX86AsmParser" .. ext,
        "LLVMX86CodeGen" .. ext,
        "LLVMX86Desc" .. ext,
        "LLVMX86Disassembler" .. ext,
        "LLVMX86Info" .. ext,
        "LLVMX86TargetMCA" .. ext,
        "LLVMXCoreCodeGen" .. ext,
        "LLVMXCoreDesc" .. ext,
        "LLVMXCoreDisassembler" .. ext,
        "LLVMXCoreInfo" .. ext,
        "LLVMXRay" .. ext,
        "LLVMipo" .. ext,
        "clangAPINotes" .. ext,
        "clangARCMigrate" .. ext,
        "clangAST" .. ext,
        "clangASTMatchers" .. ext,
        "clangAnalysis" .. ext,
        "clangAnalysisFlowSensitive" .. ext,
        "clangAnalysisFlowSensitiveModels" .. ext,
        "clangBasic" .. ext,
        "clangCodeGen" .. ext,
        "clangCrossTU" .. ext,
        "clangDependencyScanning" .. ext,
        "clangDirectoryWatcher" .. ext,
        "clangDriver" .. ext,
        "clangDynamicASTMatchers" .. ext,
        "clangEdit" .. ext,
        "clangExtractAPI" .. ext,
        "clangFormat" .. ext,
        "clangFrontend" .. ext,
        "clangFrontendTool" .. ext,
        "clangHandleCXX" .. ext,
        "clangHandleLLVM" .. ext,
        "clangIndex" .. ext,
        "clangIndexSerialization" .. ext,
        "clangInterpreter" .. ext,
        "clangLex" .. ext,
        "clangParse" .. ext,
        "clangRewrite" .. ext,
        "clangRewriteFrontend" .. ext,
        "clangSema" .. ext,
        "clangSerialization" .. ext,
        "clangStaticAnalyzerCheckers" .. ext,
        "clangStaticAnalyzerCore" .. ext,
        "clangStaticAnalyzerFrontend" .. ext,
        "clangSupport" .. ext,
        "clangTooling" .. ext,
        "clangToolingASTDiff" .. ext,
        "clangToolingCore" .. ext,
        "clangToolingInclusions" .. ext,
        "clangToolingInclusionsStdlib" .. ext,
        "clangToolingRefactoring" .. ext,
        "clangToolingSyntax" .. ext,
        "clangTransformer" .. ext
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"

    filter "platforms:Windows"
        systemversion "latest"

        links 
        {
            "advapi32.lib",
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
            "winspool.lib"
        }

    filter { "system:linux", "action:gmake" }
            buildoptions { 
                "-std=c++23"
            }

            -- Static linking of the C++ runtime to avoid issues on some Linux platforms
            linkoptions {
                "-lstdc++"
            }
    
            -- Some general libraries that might be required for a typical Linux application
            links {
                "dl",          -- dynamic linking utilities
                "pthread",     -- POSIX threads
                "m",           -- math library
                "rt"           -- real-time extensions library
            }