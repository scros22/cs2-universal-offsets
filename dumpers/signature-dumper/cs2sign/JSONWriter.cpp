#include "JSONWriter.h"
#include <sstream>
#include <iomanip>

JSONWriter::JSONWriter(const std::string& filename) 
    : m_file(filename), m_firstInObject(true), m_firstInArray(true), m_indentLevel(0) {
    if (!m_file.is_open()) {
        throw std::runtime_error("Failed to open JSON file: " + filename);
    }
}

JSONWriter::~JSONWriter() {
    Close();
}

void JSONWriter::StartObject() {
    WriteCommaIfNeeded();
    m_file << "{\n";
    m_indentLevel++;
    m_firstInObject = true;
}

void JSONWriter::EndObject() {
    m_indentLevel--;
    WriteIndent();
    m_file << "}";
    m_firstInObject = false;
}

void JSONWriter::StartArray(const std::string& name) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(name) << "\": [\n";
    m_indentLevel++;
    m_firstInArray = true;
}

void JSONWriter::EndArray() {
    m_indentLevel--;
    WriteIndent();
    m_file << "]";
    m_firstInArray = false;
}

void JSONWriter::WriteString(const std::string& key, const std::string& value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": \"" << EscapeJSON(value) << "\"";
}

void JSONWriter::WriteNumber(const std::string& key, uint64_t value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": " << value;
}

void JSONWriter::WriteBool(const std::string& key, bool value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": " << (value ? "true" : "false");
}

void JSONWriter::WriteNull(const std::string& key) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": null";
}

void JSONWriter::WriteRaw(const std::string& key, const std::string& value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": " << value;
}

void JSONWriter::StartArrayElement() {
    if (!m_firstInArray) {
        m_file << ",\n";
    }
    WriteIndent();
    m_file << "{\n";
    m_indentLevel++;
    m_firstInObject = true;
    m_firstInArray = false;
}

void JSONWriter::EndArrayElement() {
    m_indentLevel--;
    WriteIndent();
    m_file << "}";
    m_firstInObject = false;
}

void JSONWriter::WriteElementString(const std::string& key, const std::string& value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": \"" << EscapeJSON(value) << "\"";
}

void JSONWriter::WriteElementNumber(const std::string& key, uint64_t value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": " << value;
}

void JSONWriter::WriteElementBool(const std::string& key, bool value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": " << (value ? "true" : "false");
}

void JSONWriter::WriteElementNull(const std::string& key) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": null";
}

void JSONWriter::WriteElementRaw(const std::string& key, const std::string& value) {
    WriteCommaIfNeeded();
    WriteIndent();
    m_file << "\"" << EscapeJSON(key) << "\": " << value;
}

void JSONWriter::Flush() {
    m_file.flush();
}

void JSONWriter::Close() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

std::string JSONWriter::EscapeJSON(const std::string& str) {
    std::ostringstream o;
    for (size_t i = 0; i < str.length(); ++i) {
        switch (str[i]) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ('\x00' <= str[i] && str[i] <= '\x1f') {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)str[i];
                } else {
                    o << str[i];
                }
        }
    }
    return o.str();
}

void JSONWriter::WriteIndent() {
    for (int i = 0; i < m_indentLevel; ++i) {
        m_file << "  ";
    }
}

void JSONWriter::WriteCommaIfNeeded() {
    if (!m_firstInObject && !m_firstInArray) {
        m_file << ",\n";
    } else {
        m_file << "\n";
    }
}

