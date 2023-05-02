-- premake5.lua

-- get the LLVM installation path
local llvm_root = os.getenv("LLVM_ROOT")
local script_root = path.getabsolute(".")

workspace "channel"
    configurations { "Release" }
    architecture "x64"
    startproject "compiler"
    
project "compiler"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    location "compiler"
    -- dependson "external_functions"

    targetdir ("../bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("../bin-int/%{cfg.buildcfg}/%{prj.name}")

    buildoptions { 
        "/external:anglebrackets", -- handle < includes as external
        "/external:W0", -- disable warnings for external files
        "/MP" -- enable multi-processor compilation
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
        path.join(llvm_root, "Release/lib")
    }

    links 
    {
        "LLVMAArch64AsmParser.lib",
        "LLVMAArch64CodeGen.lib",
        "LLVMAArch64Desc.lib",
        "LLVMAArch64Disassembler.lib",
        "LLVMAArch64Info.lib",
        "LLVMAArch64Utils.lib",
        "LLVMAMDGPUAsmParser.lib",
        "LLVMAMDGPUCodeGen.lib",
        "LLVMAMDGPUDesc.lib",
        "LLVMAMDGPUDisassembler.lib",
        "LLVMAMDGPUInfo.lib",
        "LLVMAMDGPUTargetMCA.lib",
        "LLVMAMDGPUUtils.lib",
        "LLVMARMAsmParser.lib",
        "LLVMARMCodeGen.lib",
        "LLVMARMDesc.lib",
        "LLVMARMDisassembler.lib",
        "LLVMARMInfo.lib",
        "LLVMARMUtils.lib",
        "LLVMAVRAsmParser.lib",
        "LLVMAVRCodeGen.lib",
        "LLVMAVRDesc.lib",
        "LLVMAVRDisassembler.lib",
        "LLVMAVRInfo.lib",
        "LLVMAggressiveInstCombine.lib",
        "LLVMAnalysis.lib",
        "LLVMAsmParser.lib",
        "LLVMAsmPrinter.lib",
        "LLVMBPFAsmParser.lib",
        "LLVMBPFCodeGen.lib",
        "LLVMBPFDesc.lib",
        "LLVMBPFDisassembler.lib",
        "LLVMBPFInfo.lib",
        "LLVMBinaryFormat.lib",
        "LLVMBitReader.lib",
        "LLVMBitWriter.lib",
        "LLVMBitstreamReader.lib",
        "LLVMCFGuard.lib",
        "LLVMCodeGen.lib",
        "LLVMCore.lib",
        "LLVMCoroutines.lib",
        "LLVMCoverage.lib",
        "LLVMDWARFLinker.lib",
        "LLVMDWARFLinkerParallel.lib",
        "LLVMDWP.lib",
        "LLVMDebugInfoCodeView.lib",
        "LLVMDebugInfoDWARF.lib",
        "LLVMDebugInfoGSYM.lib",
        "LLVMDebugInfoLogicalView.lib",
        "LLVMDebugInfoMSF.lib",
        "LLVMDebugInfoPDB.lib",
        "LLVMDemangle.lib",
        "LLVMDlltoolDriver.lib",
        "LLVMExecutionEngine.lib",
        "LLVMExtensions.lib",
        "LLVMFileCheck.lib",
        "LLVMFrontendHLSL.lib",
        "LLVMFrontendOpenACC.lib",
        "LLVMFrontendOpenMP.lib",
        "LLVMFuzzMutate.lib",
        "LLVMFuzzerCLI.lib",
        "LLVMGlobalISel.lib",
        "LLVMHexagonAsmParser.lib",
        "LLVMHexagonCodeGen.lib",
        "LLVMHexagonDesc.lib",
        "LLVMHexagonDisassembler.lib",
        "LLVMHexagonInfo.lib",
        "LLVMIRPrinter.lib",
        "LLVMIRReader.lib",
        "LLVMInstCombine.lib",
        "LLVMInstrumentation.lib",
        "LLVMInterfaceStub.lib",
        "LLVMInterpreter.lib",
        "LLVMJITLink.lib",
        "LLVMLTO.lib",
        "LLVMLanaiAsmParser.lib",
        "LLVMLanaiCodeGen.lib",
        "LLVMLanaiDesc.lib",
        "LLVMLanaiDisassembler.lib",
        "LLVMLanaiInfo.lib",
        "LLVMLibDriver.lib",
        "LLVMLineEditor.lib",
        "LLVMLinker.lib",
        "LLVMLoongArchAsmParser.lib",
        "LLVMLoongArchCodeGen.lib",
        "LLVMLoongArchDesc.lib",
        "LLVMLoongArchDisassembler.lib",
        "LLVMLoongArchInfo.lib",
        "LLVMMC.lib",
        "LLVMMCA.lib",
        "LLVMMCDisassembler.lib",
        "LLVMMCJIT.lib",
        "LLVMMCParser.lib",
        "LLVMMIRParser.lib",
        "LLVMMSP430AsmParser.lib",
        "LLVMMSP430CodeGen.lib",
        "LLVMMSP430Desc.lib",
        "LLVMMSP430Disassembler.lib",
        "LLVMMSP430Info.lib",
        "LLVMMipsAsmParser.lib",
        "LLVMMipsCodeGen.lib",
        "LLVMMipsDesc.lib",
        "LLVMMipsDisassembler.lib",
        "LLVMMipsInfo.lib",
        "LLVMNVPTXCodeGen.lib",
        "LLVMNVPTXDesc.lib",
        "LLVMNVPTXInfo.lib",
        "LLVMObjCARCOpts.lib",
        "LLVMObjCopy.lib",
        "LLVMObject.lib",
        "LLVMObjectYAML.lib",
        "LLVMOption.lib",
        "LLVMOrcJIT.lib",
        "LLVMOrcShared.lib",
        "LLVMOrcTargetProcess.lib",
        "LLVMPasses.lib",
        "LLVMPowerPCAsmParser.lib",
        "LLVMPowerPCCodeGen.lib",
        "LLVMPowerPCDesc.lib",
        "LLVMPowerPCDisassembler.lib",
        "LLVMPowerPCInfo.lib",
        "LLVMProfileData.lib",
        "LLVMRISCVAsmParser.lib",
        "LLVMRISCVCodeGen.lib",
        "LLVMRISCVDesc.lib",
        "LLVMRISCVDisassembler.lib",
        "LLVMRISCVInfo.lib",
        "LLVMRISCVTargetMCA.lib",
        "LLVMRemarks.lib",
        "LLVMRuntimeDyld.lib",
        "LLVMScalarOpts.lib",
        "LLVMSelectionDAG.lib",
        "LLVMSparcAsmParser.lib",
        "LLVMSparcCodeGen.lib",
        "LLVMSparcDesc.lib",
        "LLVMSparcDisassembler.lib",
        "LLVMSparcInfo.lib",
        "LLVMSupport.lib",
        "LLVMSymbolize.lib",
        "LLVMSystemZAsmParser.lib",
        "LLVMSystemZCodeGen.lib",
        "LLVMSystemZDesc.lib",
        "LLVMSystemZDisassembler.lib",
        "LLVMSystemZInfo.lib",
        "LLVMTableGen.lib",
        "LLVMTarget.lib",
        "LLVMTargetParser.lib",
        "LLVMTextAPI.lib",
        "LLVMTransformUtils.lib",
        "LLVMVEAsmParser.lib",
        "LLVMVECodeGen.lib",
        "LLVMVEDesc.lib",
        "LLVMVEDisassembler.lib",
        "LLVMVEInfo.lib",
        "LLVMVectorize.lib",
        "LLVMWebAssemblyAsmParser.lib",
        "LLVMWebAssemblyCodeGen.lib",
        "LLVMWebAssemblyDesc.lib",
        "LLVMWebAssemblyDisassembler.lib",
        "LLVMWebAssemblyInfo.lib",
        "LLVMWebAssemblyUtils.lib",
        "LLVMWindowsDriver.lib",
        "LLVMWindowsManifest.lib",
        "LLVMX86AsmParser.lib",
        "LLVMX86CodeGen.lib",
        "LLVMX86Desc.lib",
        "LLVMX86Disassembler.lib",
        "LLVMX86Info.lib",
        "LLVMX86TargetMCA.lib",
        "LLVMXCoreCodeGen.lib",
        "LLVMXCoreDesc.lib",
        "LLVMXCoreDisassembler.lib",
        "LLVMXCoreInfo.lib",
        "LLVMXRay.lib",
        "LLVMipo.lib",
        "advapi32.lib",
        "clangAPINotes.lib",
        "clangARCMigrate.lib",
        "clangAST.lib",
        "clangASTMatchers.lib",
        "clangAnalysis.lib",
        "clangAnalysisFlowSensitive.lib",
        "clangAnalysisFlowSensitiveModels.lib",
        "clangBasic.lib",
        "clangCodeGen.lib",
        "clangCrossTU.lib",
        "clangDependencyScanning.lib",
        "clangDirectoryWatcher.lib",
        "clangDriver.lib",
        "clangDynamicASTMatchers.lib",
        "clangEdit.lib",
        "clangExtractAPI.lib",
        "clangFormat.lib",
        "clangFrontend.lib",
        "clangFrontendTool.lib",
        "clangHandleCXX.lib",
        "clangHandleLLVM.lib",
        "clangIndex.lib",
        "clangIndexSerialization.lib",
        "clangInterpreter.lib",
        "clangLex.lib",
        "clangParse.lib",
        "clangRewrite.lib",
        "clangRewriteFrontend.lib",
        "clangSema.lib",
        "clangSerialization.lib",
        "clangStaticAnalyzerCheckers.lib",
        "clangStaticAnalyzerCore.lib",
        "clangStaticAnalyzerFrontend.lib",
        "clangSupport.lib",
        "clangTooling.lib",
        "clangToolingASTDiff.lib",
        "clangToolingCore.lib",
        "clangToolingInclusions.lib",
        "clangToolingInclusionsStdlib.lib",
        "clangToolingRefactoring.lib",
        "clangToolingSyntax.lib",
        "clangTransformer.lib",
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
        --"external_functions" -- external funcs
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"

    filter "platforms:Windows"
        systemversion "latest"

-- project "external_functions"
--     kind "StaticLib"
--     language "C++"
--     cppdialect "C++20"

--     location "external_functions"

--     targetdir ("../bin/%{cfg.buildcfg}/%{prj.name}")
--     objdir ("../bin-int/%{cfg.buildcfg}/%{prj.name}")

--     files
--     {
--         "external_functions/source/**.*"
--     }

--     includedirs
--     {
--         "external_functions/source"
--     }

--     filter "configurations:Release"
--         defines { "NDEBUG" }
--         optimize "On"

--     filter "platforms:Windows"
--         systemversion "latest"

--     filter {} -- Reset the filter
--     postbuildcommands {
--         --"clang -c -emit-llvm " .. path.join(script_root, "external_functions/source/external_functions.c") .. " -o " .. path.join(script_root, "external_functions/source/external_functions.bc"),
--         --"llvm-objcopy --dump-section .llvmbc=" .. path.join(script_root, "external_functions/source/external_functions.bc.inc") .. " " .. path.join(script_root, "external_functions/source/external_functions.bc")
--     }