#ifndef HVH_WC_TOOLS_XMLHELPER_H
#define HVH_WC_TOOLS_XMLHELPER_H

#include <pugixml.hpp>

// Read an INT from an XML node
inline int readXML(pugi::xml_node parent, const char* path, int defaultValue)
{
	int result;

	pugi::xml_node child;
	if (child = parent.child(path))
		sscanf(child.text().as_string(), "%i", &result);
	else
		result = defaultValue;

	return result;
}

// Read a FLOAT from an XML node
inline float readXML(pugi::xml_node parent, const char* path, float defaultValue)
{
	float result;

	pugi::xml_node child;
	if (child = parent.child(path))
		sscanf(child.text().as_string(), "%f", &result);
	else
		result = defaultValue;

	return result;
}

// Read a STRING from an XML node
inline const char* readXML(pugi::xml_node parent, const char* path, const char* defaultValue)
{
	const char* result;

	pugi::xml_node child;
	if (child = parent.child(path))
		result = child.text().as_string();
	else
		result = defaultValue;

	return result;
}

// Read a BOOL from an XML node
inline bool readXML(pugi::xml_node parent, const char* path, bool defaultValue)
{
	bool result;

	pugi::xml_node child;
	if (child = parent.child(path))
	{
		const char* txt = child.text().as_string();
		if (strcmp(txt, "false") == 0 ||
			strcmp(txt, "0") == 0 ||
			strcmp(txt, "no") == 0)
			result = false;
		else
			result = true;
	}
	else
		result = defaultValue;

	return result;
}

// Read an INT from an XML attribute
inline int readXMLattrib(pugi::xml_node parent, const char* path, int defaultValue)
{
	int result;

	pugi::xml_attribute child;
	if (child = parent.attribute(path))
		sscanf(child.value(), "%i", &result);
	else
		result = defaultValue;

	return result;
}

// Read a FLOAT from an XML attribute
inline float readXMLattrib(pugi::xml_node parent, const char* path, float defaultValue)
{
	float result;

	pugi::xml_attribute child;
	if (child = parent.attribute(path))
		sscanf(child.value(), "%f", &result);
	else
		result = defaultValue;

	return result;
}

// Read a STRING from an XML attribute
inline const char* readXMLattrib(pugi::xml_node parent, const char* path, const char* defaultValue)
{
	const char* result;

	pugi::xml_attribute child;
	if (child = parent.attribute(path))
		result = child.value();
	else
		result = defaultValue;

	return result;
}

// Read a BOOL from an XML attribute
inline bool readXMLattrib(pugi::xml_node parent, const char* path, bool defaultValue)
{
	bool result;

	pugi::xml_attribute child;
	if (child = parent.attribute(path))
	{
		const char* txt = child.value();
		if (strcmp(txt, "false") == 0 ||
			strcmp(txt, "0") == 0 ||
			strcmp(txt, "no") == 0)
			result = false;
		else
			result = true;
	}
	else
		result = defaultValue;

	return result;
}

// Create a blank xml node unless one already exists with the same name
inline pugi::xml_node makeXMLnode(pugi::xml_document& doc, pugi::xml_node parent, const char* path)
{
	pugi::xml_node retval;
	if (retval = parent.child(path))
		return retval;
	else
		return parent.append_child(path);
}

// Write an INT to an XML node
inline void writeXML(pugi::xml_document& doc, pugi::xml_node parent, const char* path, int value)
{
	char buffer[11];
	snprintf(buffer, 11, "%i", value);

	pugi::xml_node node = parent.child(path);
	if (!node)
		node = parent.append_child(path);

	node.text() = buffer;
}

// Write a FLOAT to an XML node
inline void writeXML(pugi::xml_document& doc, pugi::xml_node parent, const char* path, float value)
{
	char buffer[32];
	snprintf(buffer, 32, "%f", value);

	pugi::xml_node node = parent.child(path);
	if (!node)
		node = parent.append_child(path);

	node.text() = buffer;
}

// Write a STRING to an XML node
inline void writeXML(pugi::xml_document& doc, pugi::xml_node parent, const char* path, const char* value)
{
	pugi::xml_node node = parent.child(path);
	if (!node)
		node = parent.append_child(path);

	node.text() = value;
}

// Write a BOOL to an XML node
inline void writeXML(pugi::xml_document& doc, pugi::xml_node parent, const char* path, bool value)
{
	char buffer[6];
	if (value == true)
		strncpy(buffer, "true", 6);
	else
		strncpy(buffer, "false", 6);

	pugi::xml_node node = parent.child(path);
	if (!node)
		node = parent.append_child(path);

	node.text() = buffer;
}

// Write an INT to an XML attribute
inline void writeXMLattrib(pugi::xml_document& doc, pugi::xml_node parent, const char* path, int value)
{
	char buffer[11];
	snprintf(buffer, 11, "%i", value);

	pugi::xml_attribute node = parent.attribute(path);
	if (!node)
		node = parent.append_attribute(path);

	node = buffer;
	//node.set_value(buffer);
}

// Write a FLOAT to an XML attribute
inline void writeXMLattrib(pugi::xml_document& doc, pugi::xml_node parent, const char* path, float value)
{
	char buffer[32];
	snprintf(buffer, 32, "%f", value);
	
	pugi::xml_attribute node = parent.attribute(path);
	if (!node)
		node = parent.append_attribute(path);

	node = buffer;
	//node.set_value(buffer);
}

// Write a STRING to an XML attribute
inline void writeXMLattrib(pugi::xml_document& doc, pugi::xml_node parent, const char* path, const char* value)
{
	pugi::xml_attribute node = parent.attribute(path);
	if (!node)
		node = parent.append_attribute(path);

	node = value;
	//node.set_value(value);
}

// Write a BOOL to an XML attribute
inline void writeXMLattrib(pugi::xml_document& doc, pugi::xml_node parent, const char* path, bool value)
{
	char buffer[6];
	if (value == true)
		strncpy(buffer, "true", 6);
	else
		strncpy(buffer, "false", 6);

	pugi::xml_attribute node = parent.attribute(path);
	if (!node)
		node = parent.append_attribute(path);

	node = buffer;
}

#endif // HVH_WC_TOOLS_XMLHELPER_H