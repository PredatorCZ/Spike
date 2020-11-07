/*  Extended pugixml API

    Copyright 2019 Lukas Cone

    Uses some drived work from puxixml (flags, enums, etc.)
    Copyright (C) 2006-2019, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#pragma once

#include "except.hpp"
#include "flags.hpp"
#include "pugixml.hpp"
#include "reflector.hpp"
#include "string_view.hpp"

REFLECTOR_CREATE(
    XMLError, ENUM, 0,
    // No error
    status_ok,
    // File was not found during load_file()
    status_file_not_found,
    // Error reading from file/stream
    status_io_error,
    // Could not allocate memory
    status_out_of_memory,
    // Internal error occurred
    status_internal_error,

    // Parser could not determine tag type
    status_unrecognized_tag,

    // Parsing error occurred while parsing document declaration/processing
    // instruction
    status_bad_pi,
    // Parsing error occurred while parsing comment
    status_bad_comment,
    // Parsing error occurred while parsing CDATA section
    status_bad_cdata,
    // Parsing error occurred while parsing document type declaration
    status_bad_doctype,
    // Parsing error occurred while parsing PCDATA section
    status_bad_pcdata,
    // Parsing error occurred while parsing start element tag
    status_bad_start_element,
    // Parsing error occurred while parsing element attribute
    status_bad_attribute,
    // Parsing error occurred while parsing end element tag
    status_bad_end_element,
    // There was a mismatch of start-end tags (closing tag had incorrect name,
    // some tag was not closed or there was an excessive closing tag)
    status_end_element_mismatch,

    // Unable to append nodes since root type is not node_element or
    // node_document (exclusive to xml_node::append_buffer)
    status_append_invalid_root,

    // Parsing resulted in a document without element nodes
    status_no_document_element);

enum class XMLParseFlag : uint32 {
  // Processing instructions (node_pi) are added to the DOM tree. [Off].
  PI,
  // Comments (node_comment) are added to the DOM tree. [Off]
  Comments,
  // CDATA sections (node_cdata) are added to the DOM tree. [On]
  CDATA,
  // Plain character data (node_pcdata) that consist only of
  // whitespace are added to the DOM tree.
  // [Off] Turning it on usually results
  // in slower parsing and more memory consumption.
  WhiteSpacePCDATA,
  // Character and entity references are expanded during parsing. [On]
  Escapes,
  // EOL characters are normalized (converted to #xA) during
  // parsing. [On]
  EOL,
  // Attribute values are normalized using CDATA normalization
  // rules during parsing. [On]
  WConvAttribute,
  // Attribute values are normalized using NMTOKENS normalization
  // rules during parsing. [Off]
  WNormAttribute,
  // Document declaration (node_declaration) is added to the DOM
  // tree. [Off]
  Declaration,
  // Document type declaration (node_doctype) is added to the DOM
  // tree. [Off]
  DocType,
  // Plain character data (node_pcdata) that is the only child of
  // the parent node and that consists only of whitespace is added to the DOM
  // tree. [Off] Turning it on may result in slower parsing and more memory
  // consumption.
  WhiteSpavePCDATASingle,
  // Leading and trailing whitespace is to be removed from plain character
  // data. [Off]
  TrimPCDATA,
  // Plain character data that does not have a parent node is
  // added to the DOM tree, and if an empty document
  // is a valid document. [Off]
  Fragment,
  // Plain character data is be stored in the parent element's
  // value. This significantly changes the structure of
  // the document; this flag is only recommended for parsing
  // documents with many PCDATA nodes in memory-constrained
  // environments. [Off]
  EmbedPCDATA,
};

using XMLParseFlags = es::Flags<XMLParseFlag>;

constexpr XMLParseFlags XMLDefaultParseFlags(XMLParseFlag::CDATA,
                                             XMLParseFlag::Escapes,
                                             XMLParseFlag::WConvAttribute,
                                             XMLParseFlag::EOL);

enum class XMLEncoding : uint32 {
  Auto,
  UTF8,
  UTF16_LE,
  UTF16_BE,
  UFT16,
  UTF32_LE,
  UTF32_be,
  UTF32,
  Wchar,
  Latin1,
};

class XMLBaseException : public std::runtime_error {
  using parent = std::runtime_error;
  static std::string Generate_(pugi::xml_node node) {
    const size_t approxHeaderSize = 25;
    auto rootNodeBegin = (*node.root().children().begin()).name();
    return "XML node " + std::string(node.name()) + " at offset " +
           std::to_string(node.name() - rootNodeBegin + approxHeaderSize) +
           ": ";
  }

public:
  XMLBaseException(const std::string &expr, pugi::xml_node node)
      : parent(Generate_(node) + expr) {}
};

class XMLMissingNodeException : public XMLBaseException {
public:
  XMLMissingNodeException(const std::string &nodeName,
                          pugi::xml_node parentNode)
      : XMLBaseException("Missing child " + nodeName, parentNode) {}
};

class XMLMissingNodeAttributeException : public XMLBaseException {
public:
  XMLMissingNodeAttributeException(const std::string &attrName,
                                   pugi::xml_node node)
      : XMLBaseException("Missing attribute " + attrName, node) {}
};

class XMLEmptyData : public XMLBaseException {
public:
  XMLEmptyData(pugi::xml_node node) : XMLBaseException("Missing data", node) {}
};

inline auto XMLCollectChildren(pugi::xml_node node,
                               es::string_view childName = {}) {
  std::vector<pugi::xml_node> children;
  std::for_each(node.begin(), node.end(), [&](auto item) {
    if (childName.empty() || item.name() == childName) {
      children.emplace_back(item);
    }
  });

  return children;
}

inline auto XMLFromFile(const std::string &fileName,
                        XMLParseFlags pflags = XMLDefaultParseFlags,
                        XMLEncoding encoding = XMLEncoding::Auto) {
  pugi::xml_document doc;
  auto result = doc.load_file(fileName.data(), static_cast<uint32>(pflags),
                              static_cast<pugi::xml_encoding>(encoding));

  if (!result) {
    if (static_cast<XMLError>(result.status) == status_file_not_found) {
      throw es::FileNotFoundError(fileName);
    }

    throw std::runtime_error(
        "Couldn't load XML file <" + fileName + ">[" +
        GetReflectedEnum<XMLError>()[result.status].to_string() +
        "] at offset " + std::to_string(result.offset));
  }

  return doc;
}
