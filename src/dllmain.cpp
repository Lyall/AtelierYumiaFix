#include "stdafx.h"
#include "helper.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <inipp/inipp.h>
#include <safetyhook.hpp>

#define spdlog_confparse(var) spdlog::info("Config Parse: {}: {}", #var, var)

HMODULE exeModule = GetModuleHandle(NULL);
HMODULE thisModule;

// Fix details
std::string sFixName = "AtelierYumiaFix";
std::string sFixVersion = "0.0.1";
std::filesystem::path sFixPath;

// Ini
inipp::Ini<char> ini;
std::string sConfigFile = sFixName + ".ini";

// Logger
std::shared_ptr<spdlog::logger> logger;
std::string sLogFile = sFixName + ".log";
std::filesystem::path sExePath;
std::string sExeName;

// Aspect ratio / FOV / HUD
std::pair DesktopDimensions = { 0,0 };
const float fPi = 3.1415926535f;
const float fNativeAspect = 16.00f / 9.00f;
float fAspectRatio;
float fAspectMultiplier;
float fHUDWidth;
float fHUDWidthOffset;
float fHUDHeight;
float fHUDHeightOffset;

// Ini variables
bool bCustomRes;
int iCustomResX;
int iCustomResY;
bool bFixHUD;

// Variables
int iCurrentResX;
int iCurrentResY;
std::uint8_t* pHUDObject = nullptr;
int iHUDObjectX;
int iHUDObjectY;

void CalculateAspectRatio(bool bLog)
{
    if (iCurrentResX <= 0 || iCurrentResY <= 0)
        return;

    // Calculate aspect ratio
    fAspectRatio = (float)iCurrentResX / (float)iCurrentResY;
    fAspectMultiplier = fAspectRatio / fNativeAspect;

    // HUD 
    fHUDWidth = (float)iCurrentResY * fNativeAspect;
    fHUDHeight = (float)iCurrentResY;
    fHUDWidthOffset = (float)(iCurrentResX - fHUDWidth) / 2.00f;
    fHUDHeightOffset = 0.00f;
    if (fAspectRatio < fNativeAspect) {
        fHUDWidth = (float)iCurrentResX;
        fHUDHeight = (float)iCurrentResX / fNativeAspect;
        fHUDWidthOffset = 0.00f;
        fHUDHeightOffset = (float)(iCurrentResY - fHUDHeight) / 2.00f;
    }

    // Log details about current resolution
    if (bLog) {
        spdlog::info("----------");
        spdlog::info("Current Resolution: Resolution: {:d}x{:d}", iCurrentResX, iCurrentResY);
        spdlog::info("Current Resolution: fAspectRatio: {}", fAspectRatio);
        spdlog::info("Current Resolution: fAspectMultiplier: {}", fAspectMultiplier);
        spdlog::info("Current Resolution: fHUDWidth: {}", fHUDWidth);
        spdlog::info("Current Resolution: fHUDHeight: {}", fHUDHeight);
        spdlog::info("Current Resolution: fHUDWidthOffset: {}", fHUDWidthOffset);
        spdlog::info("Current Resolution: fHUDHeightOffset: {}", fHUDHeightOffset);
        spdlog::info("----------");
    }
}

void Logging()
{
    // Get path to DLL
    WCHAR dllPath[_MAX_PATH] = {0};
    GetModuleFileNameW(thisModule, dllPath, MAX_PATH);
    sFixPath = dllPath;
    sFixPath = sFixPath.remove_filename();

    // Get game name and exe path
    WCHAR exePath[_MAX_PATH] = {0};
    GetModuleFileNameW(exeModule, exePath, MAX_PATH);
    sExePath = exePath;
    sExeName = sExePath.filename().string();
    sExePath = sExePath.remove_filename();

    // Spdlog initialisation
    try
    {
        logger = spdlog::basic_logger_st(sFixName, sExePath.string() + sLogFile, true);
        spdlog::set_default_logger(logger);
        spdlog::flush_on(spdlog::level::debug);

        spdlog::info("----------");
        spdlog::info("{:s} v{:s} loaded.", sFixName, sFixVersion);
        spdlog::info("----------");
        spdlog::info("Log file: {}", sFixPath.string() + sLogFile);
        spdlog::info("----------");
        spdlog::info("Module Name: {:s}", sExeName);
        spdlog::info("Module Path: {:s}", sExePath.string());
        spdlog::info("Module Address: 0x{:x}", (uintptr_t)exeModule);
        spdlog::info("Module Timestamp: {:d}", Memory::ModuleTimestamp(exeModule));
        spdlog::info("----------");
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        AllocConsole();
        FILE *dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        std::cout << "Log initialisation failed: " << ex.what() << std::endl;
        FreeLibraryAndExitThread(thisModule, 1);
    }
}

void Configuration()
{
    // Inipp initialisation
    std::ifstream iniFile(sFixPath / sConfigFile);
    if (!iniFile)
    {
        AllocConsole();
        FILE *dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        std::cout << "" << sFixName.c_str() << " v" << sFixVersion.c_str() << " loaded." << std::endl;
        std::cout << "ERROR: Could not locate config file." << std::endl;
        std::cout << "ERROR: Make sure " << sConfigFile.c_str() << " is located in " << sFixPath.string().c_str() << std::endl;
        spdlog::error("ERROR: Could not locate config file {}", sConfigFile);
        spdlog::shutdown();
        FreeLibraryAndExitThread(thisModule, 1);
    }
    else
    {
        spdlog::info("Config file: {}", sFixPath.string() + sConfigFile);
        ini.parse(iniFile);
    }

    // Parse config
    ini.strip_trailing_comments();
    spdlog::info("----------");

    // Load settings from ini
    inipp::get_value(ini.sections["Custom Resolution"], "Enabled", bCustomRes);
    inipp::get_value(ini.sections["Custom Resolution"], "Width", iCustomResX);
    inipp::get_value(ini.sections["Custom Resolution"], "Height", iCustomResY);
    inipp::get_value(ini.sections["Fix HUD"], "Enabled", bFixHUD);

    // Log ini parse
    spdlog_confparse(bCustomRes);
    spdlog_confparse(iCustomResX);
    spdlog_confparse(iCustomResY);
    spdlog_confparse(bFixHUD);

    spdlog::info("----------");
}

void CurrentResolution()
{
   // Current resolution
   std::uint8_t* CurrentResolutionScanResult = Memory::PatternScan(exeModule, "41 ?? ?? 8B ?? 48 8B ?? FF 90 ?? ?? ?? ?? 84 ?? 0F 84 ?? ?? ?? ?? 44 8B ??");
   if (CurrentResolutionScanResult) {
       spdlog::info("Current Resolution: Address is {:s}+{:x}", sExeName.c_str(), CurrentResolutionScanResult - (std::uint8_t*)exeModule);
       static SafetyHookMid CurrentResolutionMidHook{};
       CurrentResolutionMidHook = safetyhook::create_mid(CurrentResolutionScanResult,
           [](SafetyHookContext& ctx) {
               // Get current resolution
               int iResX = (int)ctx.rdx;
               int iResY = (int)ctx.r8;

               // Log resolution
               if (iResX != iCurrentResX || iResY != iCurrentResY) {
                   iCurrentResX = iResX;
                   iCurrentResY = iResY;
                   CalculateAspectRatio(true);
               }
           });
   }
   else {
       spdlog::error("Current Resolution: Pattern scan failed.");
   }
}

void Resolution()
{
    // Grab desktop resolution
    DesktopDimensions = Util::GetPhysicalDesktopDimensions();

    if (bCustomRes) 
    {
        // Set custom resolution as desktop resolution if set to 0 or invalid
        if (iCustomResX <= 0 || iCustomResY <= 0) {
            iCustomResX = DesktopDimensions.first;
            iCustomResY = DesktopDimensions.second;
        }

        // Resolution list
        std::uint8_t* ResolutionListScanResult = Memory::PatternScan(exeModule, "C0 03 00 00 1C 02 00 00 00 04 00 00 40 02 00 00");
        if (ResolutionListScanResult) {
            spdlog::info("Resolution List: Address is {:s}+{:x}", sExeName.c_str(), ResolutionListScanResult - (std::uint8_t*)exeModule);

            // Overwrite 3840x2160
            Memory::Write(ResolutionListScanResult + 0x38, iCustomResX);
            Memory::Write(ResolutionListScanResult + 0x3C, iCustomResY);
        }
        else {
            spdlog::error("Resolution List: Pattern scan failed.");
        } 

        // Resolution string
        std::uint8_t* ResolutionStringScanResult = Memory::PatternScan(exeModule, "48 85 ?? 74 ?? 48 83 ?? ?? ?? 72 ?? 48 8B ?? 48 83 ?? ?? 5B C3");
        if (ResolutionStringScanResult) {
            static bool bStringIdentified = false;

            spdlog::info("Resolution String: Address is {:s}+{:x}", sExeName.c_str(), ResolutionStringScanResult - (std::uint8_t*)exeModule);
            static SafetyHookMid ResolutionStringMidHook{};
            ResolutionStringMidHook = safetyhook::create_mid(ResolutionStringScanResult,
                [](SafetyHookContext& ctx) {
                    // This is pretty inefficient but ¯\_(ツ)_/¯
                    if (!bStringIdentified && ctx.rax) {
                    const std::string oldRes = "3840x2160";
                    std::string newRes = std::to_string(iCustomResX) + "x" + std::to_string(iCustomResY);

                    char* currentString = (char*)ctx.rax;
                    if (strncmp(currentString, oldRes.c_str(), oldRes.size()) == 0) {
                        if (newRes.size() <= oldRes.size()) {
                            std::memcpy(currentString, newRes.c_str(), newRes.size() + 1);
                            spdlog::info("Resolution String: Replaced 3840x2160 with {}", newRes);
                        }
                        bStringIdentified = true; // Stop string comparisons if we've already seen/modified "3840x2160"
                    }
                }
                });
        }
        else {
            spdlog::error("Resolution String: Pattern scan failed.");
        }
    } 

    // Log custom resolution
    iCurrentResX = iCustomResX;
    iCurrentResY = iCustomResY;
    CalculateAspectRatio(true);
}

void HUD()
{
    if (bFixHUD) 
    {
        // HUD Size
        std::uint8_t* HUDSizeScanResult = Memory::PatternScan(exeModule, "C7 ?? ?? ?? 80 07 00 00 C7 44 ?? ?? 38 04 00 00 48 89 ?? ?? ?? ?? ?? 49 8B ?? ?? ?? ?? ?? 48 85 ?? 74 ??");
        if (HUDSizeScanResult) {
            spdlog::info("HUD: Size: Address is {:s}+{:x}", sExeName.c_str(), HUDSizeScanResult - (std::uint8_t*)exeModule);
            if (fAspectRatio > fNativeAspect)
                Memory::Write(HUDSizeScanResult + 0x4, (int)ceilf(1080.00f * ((float)iCustomResX / iCustomResY)));
            else if (fAspectRatio < fNativeAspect)
                Memory::Write(HUDSizeScanResult + 0xC, (int)ceilf(1920.00f / ((float)iCustomResX / iCustomResY)));
        }
        else {
            spdlog::error("HUD: Size: Pattern scan failed.");
        }

        // HUD Objects
        std::uint8_t* HUDObjectsScanResult = Memory::PatternScan(exeModule, "89 ?? ?? 49 8B ?? ?? 48 8B ?? FF 90 ?? ?? ?? ?? 8B ?? 33 ?? 49 8B ?? ??");
        if (HUDObjectsScanResult) { 
            spdlog::info("HUD: Objects: Address is {:s}+{:x}", sExeName.c_str(), HUDObjectsScanResult - (std::uint8_t*)exeModule);
            static SafetyHookMid HUDObjectsMidHook{};
            HUDObjectsMidHook = safetyhook::create_mid(HUDObjectsScanResult,
                [](SafetyHookContext& ctx) {
                    if (ctx.r13 + 0x8) {
                        pHUDObject = *reinterpret_cast<std::uint8_t**>(ctx.r13 + 0x08);
                        iHUDObjectX = *reinterpret_cast<short*>(pHUDObject + 0xF0);
                        iHUDObjectY = *reinterpret_cast<short*>(pHUDObject + 0xF2);

                        // Backgrounds
                        if (iHUDObjectX >= 1922 && iHUDObjectY >= 1080) {
                            if (fAspectRatio > fNativeAspect)
                                ctx.rax = (static_cast<uintptr_t>(iHUDObjectY) << 16) | static_cast<short>(ceilf(iHUDObjectX * fAspectMultiplier));
                            else if (fAspectRatio < fNativeAspect)
                                ctx.rax = (static_cast<uintptr_t>(static_cast<short>(ceilf(iHUDObjectX / fAspectRatio))) << 16) | iHUDObjectX;
                        }  
                    }
                });
        }
        else {
            spdlog::error("HUD Objects: Pattern scan failed.");
        }

        /*
        // Fix cursor position in tab menu
        std::uint8_t* MenuCursorScanResult = Memory::PatternScan(exeModule, "F3 0F ?? ?? F3 0F ?? ?? F3 41 ?? ?? ?? 4C 8B ?? ?? ?? 4C 8B ?? ?? ?? ");
        if (MenuCursorScanResult) {
            spdlog::info("HUD: Cursor Position: Address is {:s}+{:x}", sExeName.c_str(), MenuCursorScanResult - (std::uint8_t*)exeModule);
            static SafetyHookMid MenuCursorHorMidHook{};
            MenuCursorHorMidHook = safetyhook::create_mid(MenuCursorScanResult,
            [](SafetyHookContext& ctx) {
                if (fAspectRatio > fNativeAspect) {
                    ctx.xmm0.f32[0] = static_cast<float>(iCurrentResX);
                    ctx.xmm6.f32[0] += fHUDWidthOffset;
                }
            });
        }
        else {
            spdlog::error("HUD: Cursor Position: Pattern scan failed.");
        }
        */
    }
}


DWORD __stdcall Main(void*)
{
    Logging();
    Configuration();
    CurrentResolution();
    Resolution();
    HUD();

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        thisModule = hModule;

        HANDLE mainHandle = CreateThread(NULL, 0, Main, 0, NULL, 0);
        if (mainHandle)
        {
            SetThreadPriority(mainHandle, THREAD_PRIORITY_HIGHEST);
            CloseHandle(mainHandle);
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
