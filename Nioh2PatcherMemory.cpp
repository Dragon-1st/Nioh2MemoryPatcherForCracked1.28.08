// Nioh2Patcher.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>

namespace GamePatchUtility {

    class CodeModification {
    private:
        int m_offset;
        std::vector<BYTE> m_replacementBytes;
        std::vector<BYTE> m_searchPattern;

    public:
        CodeModification(int offset, const std::vector<BYTE>& replacement, const std::vector<BYTE>& pattern)
            : m_offset(offset), m_replacementBytes(replacement), m_searchPattern(pattern) {
        }

        int GetOffset() const { return m_offset; }
        const std::vector<BYTE>& GetReplacement() const { return m_replacementBytes; }
        const std::vector<BYTE>& GetPattern() const { return m_searchPattern; }
    };

    class ExecutablePatcher {
    private:
        const std::wstring TARGET_EXECUTABLE = L"nioh2.exe";
        const std::wstring BACKUP_FILENAME = L"nioh2_backup.exe";

        std::vector<CodeModification> m_modifications;

    public:
        ExecutablePatcher() {
            SetupModifications();
        }

        void Execute() {
            DisplayWelcomeMessage();

            if (!CheckFileExists(TARGET_EXECUTABLE)) {
                std::wcout << L"Error: " << TARGET_EXECUTABLE << L" not found!" << std::endl;
                WaitForUserInput();
                return;
            }

            if (!CreateBackup()) {
                std::wcout << L"Warning: Could not create backup file" << std::endl;
                if (!ConfirmContinue()) {
                    return;
                }
            }

            std::vector<BYTE> fileData;
            if (!LoadFileData(TARGET_EXECUTABLE, fileData)) {
                std::wcout << L"Error: Failed to read file data" << std::endl;
                WaitForUserInput();
                return;
            }

            PerformPatching(fileData);
            WaitForUserInput();
        }

    private:
        void SetupModifications() {
            // All patch definitions from original code
            m_modifications = {
                // First patch group
                CodeModification(0xB, {0x80, 0x18, 0x55},
                               {0x48, 0x8B, 0xCA, 0xF3, 0x48, 0x0F, 0x2C, 0xC0, 0x48, 0x05, 0x00, 0x00, 0x90, 0x0C}),

                               // Graphics settings modifications
                               CodeModification(0xC, {0x20, 0x16},
                                              {0xC7, 0x44, 0x24, 0x20, 0x04, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xC0, 0x0E}),
                               CodeModification(0xC, {0xF0, 0x0F},
                                              {0xC7, 0x44, 0x24, 0x20, 0x05, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xA0, 0x0A}),
                               CodeModification(0xC, {0xA8, 0x03},
                                              {0xC7, 0x44, 0x24, 0x20, 0x06, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x70, 0x02}),
                               CodeModification(0xC, {0x48, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x07, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x30, 0x00}),
                               CodeModification(0xC, {0x78, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x08, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x50, 0x00}),
                               CodeModification(0xC, {0x78, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x09, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x50, 0x00}),
                               CodeModification(0xC, {0x20, 0x04},
                                              {0xC7, 0x44, 0x24, 0x20, 0x0A, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x10, 0x02}),
                               CodeModification(0xC, {0x0C, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x0B, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x08, 0x00}),
                               CodeModification(0xC, {0x10, 0x02},
                                              {0xC7, 0x44, 0x24, 0x20, 0x0C, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x60, 0x01}),
                               CodeModification(0xC, {0x78, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x0D, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x50, 0x00}),
                               CodeModification(0xC, {0x18, 0x0F},
                                              {0xC7, 0x44, 0x24, 0x20, 0x0E, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x10, 0x0A}),
                               CodeModification(0xC, {0xC0, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x0F, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x80, 0x00}),
                               CodeModification(0xC, {0x68, 0x01},
                                              {0xC7, 0x44, 0x24, 0x20, 0x10, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xF0, 0x00}),
                               CodeModification(0xC, {0x68, 0x01},
                                              {0xC7, 0x44, 0x24, 0x20, 0x11, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xF0, 0x00}),
                               CodeModification(0xC, {0x0C, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x12, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x08, 0x00}),
                               CodeModification(0xC, {0x08, 0x01},
                                              {0xC7, 0x44, 0x24, 0x20, 0x13, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xB0, 0x00}),
                               CodeModification(0xC, {0x48, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x14, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x30, 0x00}),
                               CodeModification(0xC, {0xD0, 0x05},
                                              {0xC7, 0x44, 0x24, 0x20, 0x15, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xE0, 0x03}),
                               CodeModification(0xC, {0xB8, 0x1A},
                                              {0xC7, 0x44, 0x24, 0x20, 0x16, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xD0, 0x11}),
                               CodeModification(0xC, {0x98, 0x1C},
                                              {0xC7, 0x44, 0x24, 0x20, 0x17, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x10, 0x13}),
                               CodeModification(0xC, {0xF0, 0x2B},
                                              {0xC7, 0x44, 0x24, 0x20, 0x18, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xF0, 0x15}),
                               CodeModification(0xC, {0x00, 0x0C},
                                              {0xC7, 0x44, 0x24, 0x20, 0x19, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x00, 0x08}),
                               CodeModification(0xC, {0xB8, 0x05},
                                              {0xC7, 0x44, 0x24, 0x20, 0x1A, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xD0, 0x03}),
                               CodeModification(0xC, {0x18, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x1B, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x10, 0x00}),
                               CodeModification(0xC, {0xF0, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x1C, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0xA0, 0x00}),
                               CodeModification(0xC, {0x18, 0x00},
                                              {0xC7, 0x44, 0x24, 0x20, 0x1D, 0x00, 0x00, 0x00, 0x41, 0xB9, 0x00, 0x00, 0x10, 0x00})
            };
        }

        void DisplayWelcomeMessage() {
            std::wcout << L"=== Nioh 2 Game Patcher ===" << std::endl;
            std::wcout << L"Version: 1.0" << std::endl;
            std::wcout << L"Purpose: Apply memory extension modifications" << std::endl;
            std::wcout << L"===========================" << std::endl;
        }

        bool CheckFileExists(const std::wstring& filename) {
            DWORD fileAttributes = GetFileAttributesW(filename.c_str());
            return (fileAttributes != INVALID_FILE_ATTRIBUTES &&
                !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
        }

        bool CreateBackup() {
            if (CheckFileExists(BACKUP_FILENAME)) {
                std::wcout << L"Backup file already exists: " << BACKUP_FILENAME << std::endl;
                return true;
            }

            if (CopyFileW(TARGET_EXECUTABLE.c_str(), BACKUP_FILENAME.c_str(), FALSE)) {
                std::wcout << L"Backup created: " << BACKUP_FILENAME << std::endl;
                return true;
            }

            return false;
        }

        bool LoadFileData(const std::wstring& filename, std::vector<BYTE>& buffer) {
            std::ifstream file(filename, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                return false;
            }

            std::streamsize fileSize = file.tellg();
            if (fileSize <= 0) {
                return false;
            }

            file.seekg(0, std::ios::beg);
            buffer.resize(static_cast<size_t>(fileSize));

            return file.read(reinterpret_cast<char*>(buffer.data()), fileSize).good();
        }

        bool SaveFileData(const std::wstring& filename, const std::vector<BYTE>& buffer) {
            std::ofstream file(filename, std::ios::binary);
            if (!file.is_open()) {
                return false;
            }

            return file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size()).good();
        }

        int LocatePattern(const std::vector<BYTE>& data, const std::vector<BYTE>& pattern, int startPos = 0) {
            if (pattern.empty() || data.size() < pattern.size()) {
                return -1;
            }

            for (size_t i = startPos; i <= data.size() - pattern.size(); ++i) {
                if (std::equal(pattern.begin(), pattern.end(), data.begin() + i)) {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }

        void DisplayByteArray(const std::vector<BYTE>& bytes) {
            std::cout << std::hex << std::uppercase;
            for (size_t i = 0; i < bytes.size(); ++i) {
                if (i > 0) std::cout << "-";
                std::cout << static_cast<int>(bytes[i]);
            }
            std::cout << std::dec;
        }

        bool ConfirmContinue() {
            std::wcout << L"Continue? (y/n): ";
            wchar_t response;
            std::wcin >> response;
            return (response == L'y' || response == L'Y');
        }

        void PerformPatching(std::vector<BYTE>& fileData) {
            std::wcout << L"\nApplying modifications..." << std::endl;

            int successfulModifications = 0;
            const size_t totalModifications = m_modifications.size();

            for (size_t i = 0; i < totalModifications; ++i) {
                const CodeModification& mod = m_modifications[i];
                int patternLocation = LocatePattern(fileData, mod.GetPattern());

                if (patternLocation == -1) {
                    std::wcout << L"[" << (i + 1) << L"/" << totalModifications
                        << L"] Pattern not found: ";
                    DisplayByteArray(mod.GetPattern());
                    std::wcout << std::endl;
                }
                else {
                    int modificationPosition = patternLocation + mod.GetOffset();
                    if (modificationPosition + mod.GetReplacement().size() <= fileData.size()) {
                        std::copy(mod.GetReplacement().begin(),
                            mod.GetReplacement().end(),
                            fileData.begin() + modificationPosition);

                        std::wcout << L"[" << (i + 1) << L"/" << totalModifications
                            << L"] Patch applied: ";
                        DisplayByteArray(mod.GetPattern());
                        std::wcout << L" -> ";
                        DisplayByteArray(mod.GetReplacement());
                        std::wcout << L" at 0x" << std::hex << modificationPosition
                            << std::dec << std::endl;
                        successfulModifications++;
                    }
                }
            }

            if (successfulModifications > 0) {
                if (SaveFileData(TARGET_EXECUTABLE, fileData)) {
                    std::wcout << L"\nSuccessfully applied " << successfulModifications
                        << L" modifications (out of " << totalModifications << L")" << std::endl;
                    std::wcout << L"Patching completed!" << std::endl;
                }
                else {
                    std::wcout << L"Error: Failed to save modified file" << std::endl;
                }
            }
            else {
                std::wcout << L"No modifications were applied" << std::endl;
            }
        }

        void WaitForUserInput() {
            std::wcout << L"\nPress Enter to exit..." << std::endl;
            std::cin.ignore();
            std::cin.get();
        }
    };

} // namespace GamePatchUtility

int main() {
    std::wcout << L"Nioh 2 Game Settings Patcher - Safe Version" << std::endl;
    std::wcout << L"This tool only modifies game memory settings, no malicious code included" << std::endl;

    GamePatchUtility::ExecutablePatcher patcher;
    patcher.Execute();

    return 0;
}