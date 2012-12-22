/*
 * xmlbind.c
 *
 *  Created on: Dec 5, 2012
 *      Author: cwikj
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "xmlbind.h"

#define PREFIX "Atmos"

char types[255][255];
int type_count=0;
FILE *headerfile, *srcfile;
char srcfilename[255];
char headerfilename[255];
char *infile;
char *outfile;
char *datestr;

static int process_type(xmlDocPtr doc, const char *name);

void AtmosUtil_log(const char *level, const char *file, const int line,
        const char *fmt, ...) {
    char msg[1024];
    char stime[255];
    time_t t;

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, 1024, fmt, args);
    va_end(args);

    // Get the current timestamp
    t = time(NULL);
    ctime_r(&t, stime);
    // ctime_r puts a newline at the end.
    stime[strlen(stime) - 1] = 0;

    fprintf(stderr, "%s - %s %s:%d: %s", stime, level, file, line, msg);
}

#define ATMOS_DEBUG(fmt, ...) do{ AtmosUtil_log("DEBUG", __FILE__, __LINE__, fmt, __VA_ARGS__); }while(0)
#define ATMOS_WARN(fmt, ...) do{ AtmosUtil_log("WARN", __FILE__, __LINE__, fmt, __VA_ARGS__); }while(0)
#define ATMOS_ERROR(fmt, ...) do{ AtmosUtil_log("ERROR", __FILE__, __LINE__, fmt, __VA_ARGS__); }while(0)


static xmlXPathObjectPtr
select_nodes(xmlDocPtr doc, xmlChar *selector) {
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;
    xmlNodeSetPtr xpathNodeSet;

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
        ATMOS_ERROR("Error: unable to create new XPath context: %s\n",
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        return NULL;
    }

    if (xmlXPathRegisterNs(xpathCtx, BAD_CAST "xsd",
                BAD_CAST "http://www.w3.org/2001/XMLSchema")) {
        ATMOS_ERROR("Error: unable to register namespace: %s\n",
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }
    if (xmlXPathRegisterNs(xpathCtx, BAD_CAST "tns",
                BAD_CAST "http://www.example.org/policy")) {
        ATMOS_ERROR("Error: unable to register namespace: %s\n",
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }

    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(selector, xpathCtx);
    if (xpathObj == NULL) {
        ATMOS_ERROR("Error: unable to evaluate xpath expression \"%s\": %s\n",
                selector,
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        xmlXPathFreeContext(xpathCtx);
        return NULL;
    }
    xpathNodeSet = xpathObj->nodesetval;
    if (!xpathNodeSet || xpathNodeSet->nodeNr == 0) {
        //ATMOS_ERROR("Error: No nodes returned from \"%s\"\n", selector);
        xmlXPathFreeContext(xpathCtx);
        xmlXPathFreeObject(xpathObj);
        return NULL;
    }

    /* Cleanup */
    xmlXPathFreeContext(xpathCtx);

    return xpathObj;
}

static void
xml_name_to_c_name(const char *xmlname, char *cname) {
    char *c;

    strcpy(cname, xmlname);

    c = cname;
    while(*c) {
        switch(*c) {
        case '-':
            *c = '_';
            break;
        case '.':
            *c = '_';
            break;
        default:
            break;
        }
        c++;
    }
}

const char*
get_supported_type_macro(const char *typeName) {
    if(!strcmp("string", typeName)) {
        return "STRING";
    } else if(!strcmp("int", typeName)) {
        return "INT";
    } else if(!strcmp("dateTime", typeName)) {
        return "DATETIME";
    } else if(!strcmp("boolean", typeName)) {
        return "BOOL";
    } else {
        return NULL;
    }
}

static void
strcatprintf(char *to, const char *fmt, ...) {
    char buffer[1024];
    buffer[0] = 0;

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 1024, fmt, args);
    va_end(args);

    strcat(to, buffer);
}

static void
get_min_max(xmlNode *node, int *minOccurs, int *maxOccurs) {
    xmlChar *minStr;
    xmlChar *maxStr;

    minStr = xmlGetProp(node, BAD_CAST "minOccurs");
    maxStr = xmlGetProp(node, BAD_CAST "maxOccurs");

    if(minStr) {
        *minOccurs = (int)strtol((char*)minStr, NULL, 10);
        xmlFree(minStr);
    } else {
        // default
        *minOccurs = 1;
    }

    if(maxStr) {
        if(!strcmp("unbounded", (char*)maxStr)) {
            *maxOccurs = -1;
        } else {
            *maxOccurs = (int)strtol((char*)maxStr, NULL, 10);
        }
        xmlFree(maxStr);
    } else {
        // default
        *maxOccurs = 1;
    }
}

static int
process_sequence(xmlDoc *doc, char *enclosing_type, xmlNode *sequence, char *constructor, char *destructor,
        char *binder, char *unbinder, char *structure) {
    xmlNode *child;
    char cname[255];

    for(child = sequence->children; child != NULL; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;  // Skip comments, etc.
        }
        if(!strcmp((char*)child->name, "element")) {
            xmlChar *type, *name;

            name = xmlGetProp(child, BAD_CAST "name");

            ATMOS_DEBUG("ELEMENT: %s/%s\n", enclosing_type, (char*)name);
            xml_name_to_c_name((char*)name, cname);

            type = xmlGetProp(child, BAD_CAST "type");
            if(!type) {
                // Probably embedded complexType
                ATMOS_ERROR("Embedded complex type not supported for %s/%s\n", enclosing_type, (char*)name);
                return 0;
            } else {
                if(strstr((char*)type, "tns:") == (char*)type) {
                    // reference to other type
                    xmlChar *typeName = type+4;
                    process_type(doc, (char*)typeName);

                    // OK, type is now defined.  Build it.
                    int minOccurs = -1;
                    int maxOccurs = -1;

                    get_min_max(child, &minOccurs, &maxOccurs);

                    if(minOccurs == 1 && maxOccurs == 1) {
                        // Single, required
                        strcatprintf(structure, "XB_STRUCT_ENTRY(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(constructor, "XB_STRUCT_ENTRY_INIT(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(destructor, "XB_STRUCT_ENTRY_FREE(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(binder, "XB_STRUCT_ENTRY_BIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                        strcatprintf(unbinder, "XB_STRUCT_ENTRY_UNBIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                    } else if(minOccurs == 0 && maxOccurs == 1) {
                        // Single, optional
                        strcatprintf(structure, "XB_STRUCT_ENTRY_OPT(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(constructor, "XB_STRUCT_ENTRY_OPT_INIT(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(destructor, "XB_STRUCT_ENTRY_OPT_FREE(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(binder, "XB_STRUCT_ENTRY_OPT_BIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                        strcatprintf(unbinder, "XB_STRUCT_ENTRY_OPT_UNBIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                    } else if(maxOccurs > 1 && minOccurs == maxOccurs) {
                        // Fixed array
                        strcatprintf(structure, "XB_STRUCT_ENTRY_FIXED_ARRAY(%s_%s, %s, %d)\n", PREFIX, typeName, cname, maxOccurs);
                        strcatprintf(constructor, "XB_STRUCT_ENTRY_FIXED_ARRAY_INIT(%s_%s, %s, %d)\n", PREFIX, typeName, cname, maxOccurs);
                        strcatprintf(destructor, "XB_STRUCT_ENTRY_FIXED_ARRAY_FREE(%s_%s, %s, %d)\n", PREFIX, typeName, cname, maxOccurs);
                        strcatprintf(binder, "XB_STRUCT_ENTRY_FIXED_ARRAY_BIND(%s_%s, %s, %s, %d)\n", PREFIX, typeName, name, cname, maxOccurs);
                        strcatprintf(unbinder, "XB_STRUCT_ENTRY_FIXED_ARRAY_UNBIND(%s_%s, %s, %s, %d)\n", PREFIX, typeName, name, cname, maxOccurs);
                    } else if(maxOccurs > 1) {
                        // Dynamic, bounded array
                        strcatprintf(structure, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(constructor, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_INIT(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(destructor, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_FREE(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(binder, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_BIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                        strcatprintf(unbinder, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_UNBIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                    } else if(maxOccurs == -1) {
                        // Dynamic, unbounded array
                        strcatprintf(structure, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(constructor, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_INIT(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(destructor, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_FREE(%s_%s, %s)\n", PREFIX, typeName, cname);
                        strcatprintf(binder, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_BIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                        strcatprintf(unbinder, "XB_STRUCT_ENTRY_DYNAMIC_ARRAY_UNBIND(%s_%s, %s, %s)\n", PREFIX, typeName, name, cname);
                    }
                } else {
                    // builtin type
                    const char *typeMacro = get_supported_type_macro((char*)type);
                    if(!typeMacro) {
                        ATMOS_WARN("Unsupported builtin type: %s\n", type);
                        return 0;
                    }
                    int minOccurs = -1;
                    int maxOccurs = -1;

                    get_min_max(child, &minOccurs, &maxOccurs);
                    if(minOccurs == 1 && maxOccurs == 1) {
                        // Single, required
                        strcatprintf(structure, "XB_%s_ENTRY(%s)\n", typeMacro, cname);
                        strcatprintf(constructor, "XB_%s_ENTRY_INIT(%s)\n", typeMacro, cname);
                        strcatprintf(destructor, "XB_%s_ENTRY_FREE(%s)\n", typeMacro, cname);
                        strcatprintf(binder, "XB_%s_ENTRY_BIND(%s, %s)\n", typeMacro, name, cname);
                        strcatprintf(unbinder, "XB_%s_ENTRY_UNBIND(%s, %s)\n", typeMacro, name, cname);
                    } else if(minOccurs == 0 && maxOccurs == 1) {
                        // Single, optional
                        strcatprintf(structure, "XB_%s_ENTRY_OPT(%s)\n", typeMacro, cname);
                        strcatprintf(constructor, "XB_%s_ENTRY_OPT_INIT(%s)\n", typeMacro, cname);
                        strcatprintf(destructor, "XB_%s_ENTRY_OPT_FREE(%s)\n", typeMacro, cname);
                        strcatprintf(binder, "XB_%s_ENTRY_OPT_BIND(%s, %s)\n", typeMacro, name, cname);
                        strcatprintf(unbinder, "XB_%s_ENTRY_OPT_UNBIND(%s, %s)\n", typeMacro, name, cname);
                    } else if(maxOccurs > 1 && minOccurs == maxOccurs) {
                        // Fixed array
                        strcatprintf(structure, "XB_%s_ENTRY_FIXED_ARRAY(%s, %d)\n", typeMacro, cname, maxOccurs);
                        strcatprintf(constructor, "XB_%s_ENTRY_FIXED_ARRAY_INIT(%s, %d)\n", typeMacro, cname, maxOccurs);
                        strcatprintf(destructor, "XB_%s_ENTRY_FIXED_ARRAY_FREE(%s, %d)\n", typeMacro, cname, maxOccurs);
                        strcatprintf(binder, "XB_%s_ENTRY_FIXED_ARRAY_BIND(%s, %s, %d)\n", typeMacro, name, cname, maxOccurs);
                        strcatprintf(unbinder, "XB_%s_ENTRY_FIXED_ARRAY_UNBIND(%s, %s, %d)\n", typeMacro, name, cname, maxOccurs);
                    } else if(maxOccurs > 1) {
                        // Dynamic, bounded array
                        strcatprintf(structure, "XB_%s_ENTRY_DYNAMIC_ARRAY(%s)\n", typeMacro, cname);
                        strcatprintf(constructor, "XB_%s_ENTRY_DYNAMIC_ARRAY_INIT(%s)\n", typeMacro, cname);
                        strcatprintf(destructor, "XB_%s_ENTRY_DYNAMIC_ARRAY_FREE(%s)\n", typeMacro, cname);
                        strcatprintf(binder, "XB_%s_ENTRY_DYNAMIC_ARRAY_BIND(%s, %s)\n", typeMacro, name, cname);
                        strcatprintf(unbinder, "XB_%s_ENTRY_DYNAMIC_ARRAY_UNBIND(%s, %s)\n", typeMacro, name, cname);
                    } else if(maxOccurs == -1) {
                        // Dynamic, unbounded array
                        strcatprintf(structure, "XB_%s_ENTRY_DYNAMIC_ARRAY(%s)\n", typeMacro, cname);
                        strcatprintf(constructor, "XB_%s_ENTRY_DYNAMIC_ARRAY_INIT(%s)\n", typeMacro, cname);
                        strcatprintf(destructor, "XB_%s_ENTRY_DYNAMIC_ARRAY_FREE(%s)\n", typeMacro, cname);
                        strcatprintf(binder, "XB_%s_ENTRY_DYNAMIC_ARRAY_BIND(%s, %s)\n", typeMacro, name, cname);
                        strcatprintf(unbinder, "XB_%s_ENTRY_DYNAMIC_ARRAY_UNBIND(%s, %s)\n", typeMacro, name, cname);
                    }

                }
                xmlFree(type);
            }
            xmlFree(name);
        } else {
            ATMOS_WARN("Unsupported type %s inside sequence", (char*)child->name);
        }
    }

    return 1;
}

static int
process_attribute(xmlNode *attribute, char *enclosing_type, char *constructor, char *destructor,
        char *binder, char *unbinder, char *structure) {

    xmlChar *name = NULL, *use = NULL, *type = NULL;
    char cname[255];
    int required = 0;

    name = xmlGetProp(attribute, BAD_CAST "name");
    use = xmlGetProp(attribute, BAD_CAST "use");
    type = xmlGetProp(attribute, BAD_CAST "type");

    ATMOS_DEBUG("ATTRIBUTE: %s/@%s\n", enclosing_type, (char*)name);
    const char *typeMacro = get_supported_type_macro((char*)type);
    if(!typeMacro) {
        ATMOS_WARN("Unsupported builtin type: %s\n", type);
        return 0;
    }

    xml_name_to_c_name((char*)name, cname);

    if(use) {
        if(!strcmp("required", (char*)use)) {
            required = 1;
        }
    }

    if(required) {
        strcatprintf(structure, "XB_%s_ENTRY(%s)\n", typeMacro, name);
        strcatprintf(constructor, "XB_%s_ENTRY_INIT(%s)\n", typeMacro, name);
        strcatprintf(destructor, "XB_%s_ENTRY_FREE(%s)\n", typeMacro, name);
        strcatprintf(binder, "XB_%s_ATTR_BIND(%s, %s)\n", typeMacro, name, cname);
        strcatprintf(unbinder, "XB_%s_ATTR_UNBIND(%s, %s)\n", typeMacro, name, cname);
    } else {
        strcatprintf(structure, "XB_%s_ENTRY_OPT(%s)\n", typeMacro, name);
        strcatprintf(constructor, "XB_%s_ENTRY_OPT_INIT(%s)\n", typeMacro, name);
        strcatprintf(destructor, "XB_%s_ENTRY_OPT_FREE(%s)\n", typeMacro, name);
        strcatprintf(binder, "XB_%s_ATTR_OPT_BIND(%s, %s)\n", typeMacro, name, cname);
        strcatprintf(unbinder, "XB_%s_ATTR_OPT_UNBIND(%s, %s)\n", typeMacro, name, cname);
    }


    xmlFree(name);
    if(use) {
        xmlFree(use);
    }
    xmlFree(type);

    return 1;
}

static int
type_processed(const char *name) {
    int i;

    for(i=0; i<type_count; i++) {
        if(!strcmp(name, types[i])) {
            return 1;
        }
    }

    // Add it now.
    strcpy(types[type_count++], name);

    return 0;
}

static int
process_type(xmlDocPtr doc, const char *name) {
    char buffer[255];
    xmlXPathObjectPtr typePtr;
    xmlNodeSetPtr typeNodeSet = NULL;

    if(type_processed(name)) {
        ATMOS_DEBUG("Type %s already processed\n", name);
        return 1;
    }

    char *constructor = calloc(1024, 1024);
    char *destructor = calloc(1024, 1024);
    char *binder = calloc(1024, 1024);
    char *unbinder = calloc(1024, 1024);
    char *structure = calloc(1024, 1024);

    snprintf(buffer, 255, "//xsd:complexType[@name=\"%s\"]", name);

    typePtr = select_nodes(doc, BAD_CAST buffer);
    if(!typePtr) {
        ATMOS_ERROR("No type found for %s: %s\n", name, buffer);
        return 0;
    }
    typeNodeSet = typePtr->nodesetval;
    xmlNode *complexTypeNode = typeNodeSet->nodeTab[0];
    xmlNode *child;

    // Start stuff
    strcat(structure, "XB_STRUCT_START\n");
    strcatprintf(constructor, "XB_CONSTRUCTOR_START(%s_%s)\n", PREFIX, name);
    strcatprintf(destructor, "XB_DESTRUCTOR_START(%s_%s)\n", PREFIX, name);
    strcatprintf(binder, "XB_BIND_START(%s_%s)\n", PREFIX, name);
    strcatprintf(unbinder, "XB_UNBIND_START(%s_%s)\n", PREFIX, name);

    for(child = complexTypeNode->children; child != NULL; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;  // Skip comments, etc.
        }
        if(!strcmp((char*)child->name, "sequence")) {
            if(!process_sequence(doc, (char*)name, child, constructor, destructor, binder, unbinder, structure)) {
                xmlXPathFreeObject(typePtr);
                return 0;
            }
        }
    }

    strcatprintf(binder, "XB_BIND_ITER_END\n");

    for(child = complexTypeNode->children; child != NULL; child=child->next) {
        if(child->type != XML_ELEMENT_NODE) {
            continue;  // Skip comments, etc.
        } else if(!strcmp((char*)child->name, "attribute")) {
            if(!process_attribute(child, (char*)name, constructor, destructor, binder, unbinder, structure)) {
                xmlXPathFreeObject(typePtr);
                return 0;
            }
        }

    }

    strcatprintf(structure, "XB_STRUCT_END(%s_%s)\n", PREFIX, name);
    strcatprintf(constructor, "XB_CONSTRUCTOR_END\n");
    strcatprintf(destructor, "XB_DESTRUCTOR_END\n");
    strcatprintf(binder, "XB_BIND_END\n");
    strcatprintf(unbinder, "XB_UNBIND_END\n");

    // Dump output
    fprintf(headerfile, "/* TYPE: %s */\n", name);
    fprintf(headerfile, "%s\n", structure);
    fprintf(headerfile, "XB_DEF_CONSTRUCTOR(%s_%s)\n", PREFIX, name);
    fprintf(headerfile, "XB_DEF_DESTRUCTOR(%s_%s)\n", PREFIX, name);
    fprintf(headerfile, "XB_DEF_UNBIND(%s_%s)\n", PREFIX, name);
    fprintf(headerfile, "XB_DEF_BIND(%s_%s)\n", PREFIX, name);
    fprintf(headerfile, "/*-----------------------------------*/\n\n");
    fprintf(srcfile, "/* TYPE: %s */\n", name);
    fprintf(srcfile, "%s\n", constructor);
    fprintf(srcfile, "%s\n", destructor);
    fprintf(srcfile, "%s\n", binder);
    fprintf(srcfile, "%s\n", unbinder);
    fprintf(srcfile, "/*-----------------------------------*/\n\n");

    free(constructor);
    free(destructor);
    free(binder);
    free(unbinder);
    free(structure);

    xmlXPathFreeObject(typePtr);
    return 1;
}

static int
process_element(xmlDocPtr doc, xmlNode *element, FILE *srcfile, FILE *headerfile) {
    xmlChar *name = NULL;
    xmlChar *type = NULL;
    xmlChar *typeName = NULL;

    // Process type
    name = xmlGetProp(element, BAD_CAST "name");
    type = xmlGetProp(element, BAD_CAST "type");

    // Type should start with "tns:"
    typeName = type+4;

    ATMOS_DEBUG("Processing element '%s', type: %s\n", (char*)name, (char*)type);

    if(!process_type(doc, (char*)typeName)) {
        ATMOS_ERROR("Failed to process type %s\n", (char*)name);
        return 0;
    }

    // Since this is a top-level element, write the serialization functions.
    fprintf(headerfile, "XB_DEF_MARSHAL(%s, " PREFIX "_%s);\n", name,
            typeName);
    fprintf(headerfile, "XB_DEF_UNMARSHAL(%s, " PREFIX "_%s);\n", name,
            typeName);
    fprintf(srcfile, "XB_MARSHAL(%s, " PREFIX "_%s)\n", name,
            typeName);
    fprintf(srcfile, "XB_UNMARSHAL(%s, " PREFIX "_%s)\n", name,
            typeName);

    xmlFree(name);
    xmlFree(type);

    return 1;
}

static void
header_start() {
    char tokenname[255];
    char *start;

    fprintf(headerfile, "/* THIS IS A GENERATED FILE.  DO NOT EDIT IT */\n\n");

    // Make a token
    start = strrchr(outfile, '/');
    if(!start) {
        start = outfile;
    }

    fprintf(headerfile, "/* %s.h - Generated by xmlbind from %s on %s */\n\n", start, infile, datestr);

    strcpy(tokenname, start);
    start = tokenname;
    while(*start) {
        if(*start == '.') {
            *start = '_';
        } else {
            *start = toupper(*start);
        }
        start++;
    }

    fprintf(headerfile, "#include \"config.h\"\n");
    fprintf(headerfile, "#include \"xmlbind_macros.h\"\n");
    fprintf(headerfile, "IGNORE_PASS #ifndef %s_H\n", tokenname);
    fprintf(headerfile, "IGNORE_PASS #define %s_H\n", tokenname);
    fprintf(headerfile, "\n\n");
    fprintf(headerfile, "IGNORE_PASS #include <string.h>\n");
    fprintf(headerfile, "IGNORE_PASS #include <time.h>\n");
    fprintf(headerfile, "IGNORE_PASS #include <stdint.h>\n");
    fprintf(headerfile, "IGNORE_PASS #include <libxml/tree.h>\n");
    fprintf(headerfile, "IGNORE_PASS #include <libxml/parser.h>\n");
    fprintf(headerfile, "\n\n");

}

static void
header_end() {
    fprintf(headerfile, "IGNORE_PASS #endif\n");
}

static void
source_start() {
    char *headerstart;

    headerstart = strrchr(outfile, '/');
    if(!headerstart) {
        headerstart = outfile;
    }

    fprintf(srcfile, "#define IGNORE_PASS\n");
    fprintf(srcfile, "#include \"config.h\"\n");
    fprintf(srcfile, "IGNORE_PASS #include \"config.h\"\n");
    fprintf(srcfile, "IGNORE_PASS #include <string.h>\n");
    fprintf(srcfile, "IGNORE_PASS #include <time.h>\n");
    fprintf(srcfile, "IGNORE_PASS #include <stdint.h>\n");
    fprintf(srcfile, "IGNORE_PASS #include <libxml/tree.h>\n");
    fprintf(srcfile, "IGNORE_PASS #include <libxml/parser.h>\n");
    fprintf(srcfile, "#include \"xmlbind_macros.h\"\n");
    fprintf(srcfile, "/* THIS IS A GENERATED FILE.  DO NOT EDIT IT */\n");
    fprintf(srcfile, "/* %s.c - Generated by xmlbind from %s on %s */\n\n", headerstart, infile, datestr);

    fprintf(srcfile, "IGNORE_PASS #include \"%s.h\"\n\n", headerstart);
    fprintf(srcfile, "DATETIME_PARSE\n\n");
}

static void
source_end() {

}

static int
xmlstart() {
    xmlDocPtr doc = NULL;
    xmlXPathObjectPtr elementPtr = NULL;
    xmlNodeSetPtr elementNodeSet = NULL;
    int entrycount;
    int i;

    doc = xmlReadFile(infile, NULL, 0);
    if(!doc) {
        ATMOS_ERROR("Error: unable to read doc \"%s\": %s\n",
                infile,
                xmlGetLastError()?xmlGetLastError()->message:"(null)");
        return -1;
    }

    snprintf(srcfilename, 255, "%s.xb.c", outfile);
    srcfile = fopen(srcfilename, "w");
    if(!srcfile) {
        ATMOS_ERROR("Could not open %s for write\n", srcfile);
        xmlFreeDoc(doc);
        return -3;
    }
    snprintf(headerfilename, 255, "%s.xb.h", outfile);
    headerfile = fopen(headerfilename, "w");
    if(!headerfile) {
        ATMOS_ERROR("Could not open %s for write\n", srcfile);
        xmlFreeDoc(doc);
        return -3;
    }

    time_t now = time(NULL);
    datestr = ctime(&now);
    // Strip trailing \n
    datestr[strlen(datestr)-1] = 0;

    header_start();
    source_start();

    // Get root elements
    elementPtr = select_nodes(doc, BAD_CAST "/xsd:schema/xsd:element");
    if(!elementPtr) {
        ATMOS_ERROR("No elements found in XSD: %s\n", "//xsd:element");
        xmlFreeDoc(doc);
        return -2;
    }

    elementNodeSet = elementPtr->nodesetval;
    entrycount = elementNodeSet->nodeNr;
    if(entrycount == 0) {
        // Empty directory
        xmlXPathFreeObject(elementPtr);
        xmlFreeDoc(doc);
        return -2;
    }

    // Iterate through entries
    for(i=0; i<entrycount; i++) {
        xmlNode *entrynode = elementNodeSet->nodeTab[i];
        if(!process_element(doc, entrynode, srcfile, headerfile)) {
            xmlXPathFreeObject(elementPtr);
            xmlFreeDoc(doc);
            return -3;
        }
    }

    header_end();
    source_end();

    // Cleanup
    xmlXPathFreeObject(elementPtr);
    xmlFreeDoc(doc);

    fclose(srcfile);
    fclose(headerfile);
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: xmlbind {infile.xsd} {outfile}\n");
        return -1;
    }

    infile = argv[1];
    outfile = argv[2];

    return xmlstart();
}
