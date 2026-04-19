#pragma once

#include <string>
#include <fstream>
#include <vector>

class JSONWriter {
public:
    JSONWriter(const std::string& filename);
    ~JSONWriter();
    
    void StartObject();
    void EndObject();
    void StartArray(const std::string& name);
    void EndArray();
    void WriteString(const std::string& key, const std::string& value);
    void WriteNumber(const std::string& key, uint64_t value);
    void WriteBool(const std::string& key, bool value);
    void WriteNull(const std::string& key);
    void WriteRaw(const std::string& key, const std::string& value);
    
    void StartArrayElement();
    void EndArrayElement();
    void WriteElementString(const std::string& key, const std::string& value);
    void WriteElementNumber(const std::string& key, uint64_t value);
    void WriteElementBool(const std::string& key, bool value);
    void WriteElementNull(const std::string& key);
    void WriteElementRaw(const std::string& key, const std::string& value);
    
    void Flush();
    void Close();
    
    static std::string EscapeJSON(const std::string& str);

private:
    std::ofstream m_file;
    bool m_firstInObject;
    bool m_firstInArray;
    int m_indentLevel;
    
    void WriteIndent();
    void WriteCommaIfNeeded();
};

