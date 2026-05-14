#ifndef __PROGTEST__

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <optional>
#include <memory>
#include <stdexcept>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#endif /* __PROGTEST__ */

class CLinker {
public:
    CLinker() = default;

    ~CLinker() = default;

    CLinker(const CLinker &) = delete;

    CLinker &operator=(const CLinker &) = delete;

    CLinker &addFile(const std::string &m_fileName) {
        try {
            ObjectFile objFile;
            readObjFile(m_fileName, objFile);
            m_objFiles.push_back(std::move(objFile));
        } catch (const std::exception &e) {
            throw std::runtime_error("read fail");
        }
        return *this;
    }

    void linkOutput(const std::string &m_fileName, const std::string &ep) {
        if (m_objFiles.empty())
            throw std::runtime_error("no file");

        globalSymbTableBuildFunc();

        if (m_symbTable.find(ep) == m_symbTable.end())
            throw std::runtime_error("fail --- " + ep);

        std::vector<std::string> funcs;
        std::set<std::string> visited;
        reachFuncs(ep, funcs, visited);

        outputFileGenFunc(m_fileName, ep, funcs);
    }

private:

    struct SymbolEntry {
        size_t m_objIndex;
        size_t m_expTableIndex;
    };
    struct ExportedFunction {
        std::string m_name;
        uint32_t m_offset;
    };
    struct ImportReference {
        std::string m_name;
        std::vector<uint32_t> m_offsets;
    };
    struct ObjectFile {
        std::string m_fileName;
        std::vector<ExportedFunction> m_expTable;
        std::vector<ImportReference> m_impTable;
        std::vector<uint8_t> m_compiled;
    };


    std::vector<ObjectFile> m_objFiles;
    std::unordered_map<std::string, SymbolEntry> m_symbTable;

    void readObjFile(const std::string &m_fileName, ObjectFile &objFile) {
        std::ifstream file(m_fileName, std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("fail open -----" + m_fileName);

        objFile.m_fileName = m_fileName;

        // Read header
        uint32_t exportCount, importCount, codeSize;
        file.read(reinterpret_cast<char *>(&exportCount), sizeof(exportCount));
        file.read(reinterpret_cast<char *>(&importCount), sizeof(importCount));
        file.read(reinterpret_cast<char *>(&codeSize), sizeof(codeSize));

        if (!file.good())
            throw std::runtime_error("readObjFile error 1");

        for (uint32_t i = 0; i < exportCount; ++i) {
            ExportedFunction exportFunc;
            uint8_t nameLength;
            file.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
            exportFunc.m_name.resize(nameLength);
            file.read(&exportFunc.m_name[0], nameLength);
            file.read(reinterpret_cast<char *>(&exportFunc.m_offset), sizeof(exportFunc.m_offset));

            if (!file.good())
                throw std::runtime_error("readObjFile ExportedFunction error");

            objFile.m_expTable.push_back(exportFunc);
        }

        for (uint32_t i = 0; i < importCount; ++i) {
            ImportReference importRef;

            uint8_t nameLength;
            file.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
            importRef.m_name.resize(nameLength);
            file.read(&importRef.m_name[0], nameLength);
            uint32_t refCount;
            file.read(reinterpret_cast<char *>(&refCount), sizeof(refCount));
            importRef.m_offsets.resize(refCount);
            for (uint32_t j = 0; j < refCount; ++j) {
                file.read(reinterpret_cast<char *>(&importRef.m_offsets[j]), sizeof(uint32_t));
            }

            if (!file.good())
                throw std::runtime_error("import table error");

            objFile.m_impTable.push_back(importRef);
        }

        objFile.m_compiled.resize(codeSize);
        file.read(reinterpret_cast<char *>(objFile.m_compiled.data()), codeSize);
        if (!file.good() || file.gcount() != static_cast<std::streamsize>(codeSize))
            throw std::runtime_error("compiled code error");
    }

    void reachFuncs(const std::string &startFunc,
                    std::vector<std::string> &reachableFuncs,
                    std::set<std::string> &visited) {
        if (visited.count(startFunc) > 0)
            return;  // visited

        visited.insert(startFunc);
        reachableFuncs.push_back(startFunc);
        const auto &symEntry = m_symbTable[startFunc];
        const auto &objFile = m_objFiles[symEntry.m_objIndex];

        // all imported functions by this function
        for (const auto &importRef: objFile.m_impTable) {
            const uint32_t functionOffset = objFile.m_expTable[symEntry.m_expTableIndex].m_offset;
            uint32_t functionEndOffset = objFile.m_compiled.size();
            //  end m_offset of the current function
            for (const auto &exportFunc: objFile.m_expTable) {
                if (exportFunc.m_offset > functionOffset && exportFunc.m_offset < functionEndOffset) {
                    functionEndOffset = exportFunc.m_offset;
                }
            }
            bool isReferencedByCurrentFunc = false;
            for (uint32_t refOffset: importRef.m_offsets) {
                if (refOffset >= functionOffset && refOffset < functionEndOffset) {
                    isReferencedByCurrentFunc = true;
                    break;
                }
            }
            if (isReferencedByCurrentFunc) {
                if (m_symbTable.find(importRef.m_name) == m_symbTable.end()) {
                    throw std::runtime_error("Undefined symbol " + importRef.m_name);
                }

                reachFuncs(importRef.m_name, reachableFuncs, visited);
            }
        }
    }

    void globalSymbTableBuildFunc() {
        m_symbTable.clear();

        for (size_t fileIdx = 0; fileIdx < m_objFiles.size(); ++fileIdx) {
            const auto &objFile = m_objFiles[fileIdx];
            for (size_t expIdx = 0; expIdx < objFile.m_expTable.size(); ++expIdx) {
                const auto &func = objFile.m_expTable[expIdx];
                // Check for duplicate symbol
                auto it = m_symbTable.find(func.m_name);
                if (it != m_symbTable.end()) {
                    throw std::runtime_error("Duplicate symbol: " + func.m_name);
                }
                m_symbTable[func.m_name] = {fileIdx, expIdx};
            }
        }
    }


    void outputFileGenFunc(const std::string &m_fileName,
                           const std::string &ep,
                           const std::vector<std::string> &reachableFuncs) {
        std::ofstream outFile(m_fileName, std::ios::binary);
        if (!outFile.is_open())
            throw std::runtime_error("output file creation error: " + m_fileName);

        std::unordered_map<std::string, uint32_t> funcPos;
        uint32_t curPos = 0;
        const auto &entrySymEntry = m_symbTable[ep];
        const auto &entryObjFile = m_objFiles[entrySymEntry.m_objIndex];
        const auto &entryExport = entryObjFile.m_expTable[entrySymEntry.m_expTableIndex];
        uint32_t entryEndOffset = entryObjFile.m_compiled.size();
        for (const auto &exportFunc: entryObjFile.m_expTable) {
            if (exportFunc.m_offset > entryExport.m_offset && exportFunc.m_offset < entryEndOffset) {
                entryEndOffset = exportFunc.m_offset;
            }
        }
        uint32_t entrySize = entryEndOffset - entryExport.m_offset;

        funcPos[ep] = 0;
        curPos += entrySize;
        std::vector<std::string> sortedFuncs(reachableFuncs.begin(), reachableFuncs.end());
        std::sort(sortedFuncs.begin(), sortedFuncs.end());

        for (const auto &funcName: sortedFuncs) {
            if (funcName == ep)
                continue;

            const auto &symEntry = m_symbTable[funcName];
            const auto &objFile = m_objFiles[symEntry.m_objIndex];
            const auto &exportFunc = objFile.m_expTable[symEntry.m_expTableIndex];
            uint32_t funcEndOffset = objFile.m_compiled.size();
            for (const auto &expFunc: objFile.m_expTable) {
                if (expFunc.m_offset > exportFunc.m_offset && expFunc.m_offset < funcEndOffset) {
                    funcEndOffset = expFunc.m_offset;
                }
            }
            uint32_t funcSize = funcEndOffset - exportFunc.m_offset;
            funcPos[funcName] = curPos;
            curPos += funcSize;
        }

        // write all functions to the output file with corrected addresses
        {
            // Write entry point function first
            const auto &entrySymEntry = m_symbTable[ep];
            const auto &entryObjFile = m_objFiles[entrySymEntry.m_objIndex];
            const auto &entryExport = entryObjFile.m_expTable[entrySymEntry.m_expTableIndex];

            std::vector<uint8_t> functionCode(entryObjFile.m_compiled.begin() + entryExport.m_offset,
                                              entryObjFile.m_compiled.begin() + entryExport.m_offset + entrySize);

            for (const auto &importRef: entryObjFile.m_impTable) {
                for (uint32_t m_offset: importRef.m_offsets) {
                    if (m_offset >= entryExport.m_offset && m_offset < entryExport.m_offset + entrySize) {
                        uint32_t relativeOffset = m_offset - entryExport.m_offset;
                        uint32_t targetPos = funcPos[importRef.m_name];
                        memcpy(&functionCode[relativeOffset], &targetPos, sizeof(uint32_t));
                    }
                }
            }

            outFile.write(reinterpret_cast<const char *>(functionCode.data()), functionCode.size());
            if (!outFile.good())
                throw std::runtime_error("writing error 1");
        }

        for (const auto &funcName: sortedFuncs) {
            if (funcName == ep)
                continue;

            const auto &symEntry = m_symbTable[funcName];
            const auto &objFile = m_objFiles[symEntry.m_objIndex];
            const auto &exportFunc = objFile.m_expTable[symEntry.m_expTableIndex];
            uint32_t funcEndOffset = objFile.m_compiled.size();
            for (const auto &expFunc: objFile.m_expTable) {
                if (expFunc.m_offset > exportFunc.m_offset && expFunc.m_offset < funcEndOffset) {
                    funcEndOffset = expFunc.m_offset;
                }
            }
            uint32_t funcSize = funcEndOffset - exportFunc.m_offset;
            std::vector<uint8_t> functionCode(objFile.m_compiled.begin() + exportFunc.m_offset,
                                              objFile.m_compiled.begin() + exportFunc.m_offset + funcSize);

            for (const auto &importRef: objFile.m_impTable) {
                for (uint32_t m_offset: importRef.m_offsets) {
                    if (m_offset >= exportFunc.m_offset && m_offset < exportFunc.m_offset + funcSize) {
                        uint32_t relativeOffset = m_offset - exportFunc.m_offset;
                        uint32_t targetPos = funcPos[importRef.m_name];
                        memcpy(&functionCode[relativeOffset], &targetPos, sizeof(uint32_t));
                    }
                }
            }
            outFile.write(reinterpret_cast<const char *>(functionCode.data()), functionCode.size());

            if (!outFile.good())
                throw std::runtime_error("writing error 2");
        }
    }
};

#ifndef __PROGTEST__

int main() {
    CLinker().addFile("0in0.o").linkOutput("0out", "strlen");

    CLinker().addFile("1in0.o").linkOutput("1out", "main");

    CLinker().addFile("2in0.o").addFile("2in1.o").linkOutput("2out", "main");

    CLinker().addFile("3in0.o").addFile("3in1.o").linkOutput("3out", "towersOfHanoi");

    try {
        CLinker().addFile("4in0.o").addFile("4in1.o").linkOutput("4out", "unusedFunc");
        assert ("missing an exception" == nullptr);
    }
    catch (const std::runtime_error &e) {
        // e . what (): Undefined symbol qsort
    }
    catch (...) {
        assert ("invalid exception" == nullptr);
    }

    try {
        CLinker().addFile("5in0.o").linkOutput("5out", "main");
        assert ("missing an exception" == nullptr);
    }
    catch (const std::runtime_error &e) {
        // e . what (): Duplicate symbol: printf
    }
    catch (...) {
        assert ("invalid exception" == nullptr);
    }

    try {
        CLinker().addFile("6in0.o").linkOutput("6out", "strlen");
        assert ("missing an exception" == nullptr);
    }
    catch (const std::runtime_error &e) {
        // e . what (): read fail
    }
    catch (...) {
        assert ("invalid exception" == nullptr);
    }

    try {
        CLinker().addFile("7in0.o").linkOutput("7out", "strlen2");
        assert ("missing an exception" == nullptr);
    }
    catch (const std::runtime_error &e) {
        // e . what (): Undefined symbol strlen2
    }
    catch (...) {
        assert ("invalid exception" == nullptr);
    }

    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
