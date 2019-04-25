/*	Pugixml extension header
	more info in README for PreCore Project

	Copyright 2019 Lukas Cone

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

REFLECTOR_ENUM_NAKED(XMLError,
	status_ok,				// No error

	status_file_not_found,		// File was not found during load_file()
	status_io_error,			// Error reading from file/stream
	status_out_of_memory,		// Could not allocate memory
	status_internal_error,		// Internal error occurred

	status_unrecognized_tag,	// Parser could not determine tag type

	status_bad_pi,				// Parsing error occurred while parsing document declaration/processing instruction
	status_bad_comment,			// Parsing error occurred while parsing comment
	status_bad_cdata,			// Parsing error occurred while parsing CDATA section
	status_bad_doctype,			// Parsing error occurred while parsing document type declaration
	status_bad_pcdata,			// Parsing error occurred while parsing PCDATA section
	status_bad_start_element,	// Parsing error occurred while parsing start element tag
	status_bad_attribute,		// Parsing error occurred while parsing element attribute
	status_bad_end_element,		// Parsing error occurred while parsing end element tag
	status_end_element_mismatch,// There was a mismatch of start-end tags (closing tag had incorrect name, some tag was not closed or there was an excessive closing tag)

	status_append_invalid_root,	// Unable to append nodes since root type is not node_element or node_document (exclusive to xml_node::append_buffer)

	status_no_document_element	// Parsing resulted in a document without element nodes
);

#define GetXMLErrorMessage(var) _EnumWrap<XMLError>{}._reflected[var.status]
