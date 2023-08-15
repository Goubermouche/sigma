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

function concat_tables(t1,t2)
    for i=1,#t2 do
        t1[#t1+1] = t2[i]
    end
    return t1
end

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
        "clangTransformer",

        "Version"
    } 
    
    links(concat_tables(llvm_libs_table, clang_libs))

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