function split(inputstr, sep)
    if sep == nil then
        sep = "%s" -- default to splitting on spaces
    end
    local t = {}
    for str in string.gmatch(inputstr, "([^" .. sep .. "]+)") do
        table.insert(t, str)
    end
    return t
end

-- get the LLVM installation path
local llvm_root = os.getenv("LLVM_ROOT")
local script_root = path.getabsolute(".")
print("LLVM_ROOT: " .. llvm_root)

workspace "sigma"
    configurations { "Release" }
    architecture "x64"
    startproject "compiler"

    language "C++"
    cppdialect "C++latest"

    externalanglebrackets "On"
    externalwarnings "Off"

    flags {
        "MultiProcessorCompile"
    }

-- utility 
project "utility"
    kind "StaticLib"
    location "utility"
    targetdir "bin/%{cfg.buildcfg}"

    files {
        "utility/source/**.h",
        "utility/source/**.cpp"
    } 

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"

    includedirs {
        "utility/source",
        path.join(llvm_root, "include")
    }

    libdirs {   
        path.join(llvm_root, "lib"),
        path.join(llvm_root, "bin"),
        path.join(llvm_root, "../llvm-build/lib"),
        path.join(llvm_root, "Release/lib")
    }

    local llvm_libs = os.outputof("llvm-config --libs support core")
    local llvm_libs_table = split(llvm_libs)

    links {
        llvm_libs_table
    }

-- compiler
project "compiler"
    kind "ConsoleApp"
    location "compiler"
    targetdir "bin/%{cfg.buildcfg}"

    files {
        "compiler/source/**.*",
        "compiler/main.cpp"
    }

    includedirs {
        "compiler/source",
        "utility/source",
        path.join(llvm_root, "include")
    }

    libdirs {   
        path.join(llvm_root, "lib"),
        path.join(llvm_root, "bin"),
        path.join(llvm_root, "../llvm-build/lib"),
        path.join(llvm_root, "Release/lib")
    }

    local llvm_libs = os.outputof("llvm-config --libs all")
    local llvm_libs_table = split(llvm_libs)

    local clang_libs = { 
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
    
    links {
        llvm_libs_table,
        clang_libs,
        "Version",

        "utility"
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"
        
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