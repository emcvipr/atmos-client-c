/*
 * xmlbind_macros.h
 *
 *  Created on: Dec 5, 2012
 *      Author: cwikj
 */

#ifndef XMLBIND_MACROS_H_
#define XMLBIND_MACROS_H_

/***********************************************************************
 * Headers                                                             *
 * We do this so the first pass of the CPP doesn't inline the includes *
 ***********************************************************************/

#define IGNORE_PASS

/********************
 * Helper functions *
 ********************/

#define DATETIME_PARSE \
    static time_t datetime_parse(xmlChar *value) {\
        time_t tt; \
        struct tm t; \
        t.tm_gmtoff=0; \
        char buffer[255]; \
        char *decimal; \
        size_t strsz; \
        strsz = strlen((char*)value);\
        /* strptime doesn't accept "Z" as GMT.*/\
        if(value[strsz-1] == 'Z') {\
            strlcpy(buffer, (char*)value, 255); \
            buffer[strlen(buffer)-1] = 0;\
            /** Also remove milliseconds if it's there */ \
            decimal = strchr(buffer, '.'); \
            if(decimal) { \
                *decimal = 0;\
            }\
            strlcat(buffer, "GMT", 255);\
            if(!strptime(buffer, "%FT%T%Z", &t)) { \
                fprintf(stderr, "Could not parse dateTime value %s\n", buffer); \
                return 0; \
            }\
            tt = mktime(&t);\
            return tt;\
        } else if(value[strsz-5] == '+' || value[strsz-5] == '-') {\
            /* RFC822 TZ (+-XXXX) is OK.*/\
            if(!strptime((char*)value, "%FT%T%z", &t)) { \
                fprintf(stderr, "Could not parse dateTime value %s\n", buffer); \
                return 0; \
            }\
            tt = mktime(&t);\
            return tt;\
        } else {\
            /** Unknown.  Try it as-is with named TZ. */\
            if(!strptime((char*)value, "%FT%T%Z", &t)) { \
                fprintf(stderr, "Could not parse dateTime value %s\n", buffer); \
                return 0; \
            }\
            tt = mktime(&t);\
            return tt;\
        }\
   }\


/*********************************
 * top-level element marshallers *
 *********************************/
#define XB_DEF_MARSHAL(XML_NAME, STRUCT_NAME) \
    xmlChar* STRUCT_NAME ## _marshal(STRUCT_NAME *data);

#define XB_DEF_UNMARSHAL(XML_NAME, STRUCT_NAME) \
    STRUCT_NAME* STRUCT_NAME ## _unmarshal(const char *xml);

#define XB_MARSHAL(XML_NAME, STRUCT_NAME) \
    xmlChar* STRUCT_NAME ## _marshal(STRUCT_NAME *data) { \
    xmlNodePtr n; \
    xmlDocPtr doc; \
    xmlChar *xmlbuff; \
    int buffersize; \
    \
    doc = xmlNewDoc(BAD_CAST "1.0"); \
    n = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
    STRUCT_NAME ## _unbind(data, n); \
    \
    xmlDocSetRootElement(doc, n); \
    \
    xmlDocDumpFormatMemoryEnc(doc, &xmlbuff, &buffersize, "UTF-8", 1); \
    \
    xmlFreeDoc(doc); \
    return(xmlbuff); \
}

#define XB_UNMARSHAL(XML_NAME, STRUCT_NAME) \
    STRUCT_NAME* STRUCT_NAME ## _unmarshal(const char *xml) {\
    STRUCT_NAME* data; \
    xmlDocPtr doc; \
    xmlNodePtr root; \
    int docsz; \
    \
    data = STRUCT_NAME ## _init(malloc(sizeof(STRUCT_NAME))); \
    \
    docsz = (int)strlen(xml); \
    doc = xmlReadMemory(xml, docsz, "noname.xml", NULL, 0); \
    if(doc == NULL) { \
        STRUCT_NAME ## _destroy(data); \
        free(data); \
        return NULL; \
    } \
    root = xmlDocGetRootElement(doc); \
    \
    STRUCT_NAME ## _bind(data, root); \
    \
    xmlFreeDoc(doc);\
    return data; \
    }


/********************
 * Structure macros *
 ********************/
#define XB_STRUCT_START typedef struct {
#define XB_STRUCT_END(STRUCT_NAME) } STRUCT_NAME;
#define XB_INT_ENTRY(NAME) long long NAME;
#define XB_INT_ENTRY_OPT(NAME) long long NAME; int NAME ## _set;
#define XB_INT_ENTRY_FIXED_ARRAY(NAME, SIZE) long long NAME[SIZE]; int NAME ## _i;
#define XB_INT_ENTRY_DYNAMIC_ARRAY(NAME) long long *NAME; int NAME ## _count;
#define XB_STRING_ENTRY(NAME) char *NAME;
#define XB_STRING_ENTRY_OPT(NAME) char *NAME;
#define XB_STRING_ENTRY_FIXED_ARRAY(NAME, SIZE) char *NAME[SIZE]; int NAME ## _i;
#define XB_STRING_ENTRY_DYNAMIC_ARRAY(NAME) char **NAME; int NAME ## _count;
#define XB_BOOL_ENTRY(NAME) int NAME;
#define XB_BOOL_ENTRY_OPT(NAME) int NAME; int NAME ## _set;
#define XB_BOOL_ENTRY_FIXED_ARRAY(NAME, SIZE) int NAME[SIZE]; int NAME ## _i;
#define XB_BOOL_ENTRY_DYNAMIC_ARRAY(NAME) int *NAME; int NAME ## _count;
#define XB_DATETIME_ENTRY(NAME) time_t NAME;
#define XB_DATETIME_ENTRY_OPT(NAME) time_t NAME; int NAME ## _set;
#define XB_DATETIME_ENTRY_FIXED_ARRAY(NAME, SIZE) time_t NAME[SIZE]; int NAME ## _i;
#define XB_DATETIME_ENTRY_DYNAMIC_ARRAY(NAME) time_t *NAME; int NAME ## _count;
#define XB_STRUCT_ENTRY(STRUCT_TYPE, NAME) STRUCT_TYPE NAME;
#define XB_STRUCT_ENTRY_OPT(STRUCT_TYPE, NAME) STRUCT_TYPE *NAME;
#define XB_STRUCT_ENTRY_FIXED_ARRAY(STRUCT_TYPE, NAME, SIZE) STRUCT_TYPE NAME[SIZE]; int NAME ## _i;
#define XB_STRUCT_ENTRY_DYNAMIC_ARRAY(STRUCT_TYPE, NAME) STRUCT_TYPE *NAME; int NAME ## _count;

/**********************
 * Constructor Macros *
 **********************/
#define XB_DEF_CONSTRUCTOR(STRUCT_TYPE) \
        STRUCT_TYPE* STRUCT_TYPE ## _init(STRUCT_TYPE *self);
#define XB_CONSTRUCTOR_START(STRUCT_TYPE) \
        STRUCT_TYPE* STRUCT_TYPE ## _init(STRUCT_TYPE *self) { \
    memset(self, 0, sizeof(STRUCT_TYPE));
#define XB_CONSTRUCTOR_END return self; }
#define XB_STRUCT_ENTRY_INIT(STRUCT_TYPE, NAME) /* no-op */
#define XB_STRUCT_ENTRY_OPT_INIT(STRUCT_TYPE, NAME) /* no-op */
#define XB_STRUCT_ENTRY_FIXED_ARRAY_INIT(STRUCT_TYPE, NAME, COUNT) \
{ int i; for(i=0; i<COUNT; i++) { STRUCT_TYPE ## _init(&self->NAME[i]); }}
#define XB_STRUCT_ENTRY_DYNAMIC_ARRAY_INIT(STRUCT_TYPE, NAME) /* no-op */
#define XB_INT_ENTRY_INIT(NAME) /* no-op */
#define XB_STRING_ENTRY_INIT(NAME) /* no-op */
#define XB_BOOL_ENTRY_INIT(NAME) /* no-op */
#define XB_DATETIME_ENTRY_INIT(NAME) self->NAME=-1;
#define XB_INT_ENTRY_OPT_INIT(NAME) /* no-op */
#define XB_STRING_ENTRY_OPT_INIT(NAME) /* no-op */
#define XB_BOOL_ENTRY_OPT_INIT(NAME) /* no-op */
#define XB_DATETIME_ENTRY_OPT_INIT(NAME) /* no-op */
#define XB_INT_ENTRY_FIXED_ARRAY_INIT(NAME, COUNT) /* no-op */
#define XB_STRING_ENTRY_FIXED_ARRAY_INIT(NAME, COUNT) /* no-op */
#define XB_BOOL_ENTRY_FIXED_ARRAY_INIT(NAME, COUNT) /* no-op */
#define XB_DATETIME_ENTRY_FIXED_ARRAY_INIT(NAME, COUNT) { int i; for(i=0; i<COUNT; i++) { self->NAME[i] = -1; } }
#define XB_INT_ENTRY_DYNAMIC_ARRAY_INIT(NAME) /* no-op */
#define XB_STRING_ENTRY_DYNAMIC_ARRAY_INIT(NAME) /* no-op */
#define XB_BOOL_ENTRY_DYNAMIC_ARRAY_INIT(NAME) /* no-op */
#define XB_DATETIME_ENTRY_DYNAMIC_ARRAY_INIT(NAME) /* no-op */

/*********************
 * Destructor Macros *
 *********************/
#define XB_DEF_DESTRUCTOR(STRUCT_TYPE) \
        void STRUCT_TYPE ## _destroy(STRUCT_TYPE *self);
#define XB_DESTRUCTOR_START(STRUCT_TYPE) \
        void STRUCT_TYPE ## _destroy(STRUCT_TYPE *self) {
#define XB_DESTRUCTOR_END }
#define XB_STRUCT_ENTRY_FREE(STRUCT_TYPE, NAME) \
        STRUCT_TYPE ## _destroy(&self->NAME);
#define XB_STRUCT_ENTRY_OPT_FREE(STRUCT_TYPE, NAME) \
        if(self->NAME) { STRUCT_TYPE ## _destroy(self->NAME); free(self->NAME); self->NAME = NULL; }
#define XB_STRUCT_ENTRY_FIXED_ARRAY_FREE(STRUCT_NAME, NAME, COUNT) \
        { int i; for(i=0; i<COUNT; i++) { STRUCT_TYPE ## _destroy(&self->NAME[i]); }}
#define XB_STRUCT_ENTRY_DYNAMIC_ARRAY_FREE(STRUCT_NAME, NAME) \
        if(self->NAME) { int i; for(i=0; i< self->NAME ## _count; i++) { \
            STRUCT_NAME ## _destroy(&self->NAME[i]); } free(self->NAME); self->NAME = NULL; }
#define XB_INT_ENTRY_FREE(NAME) self->NAME = 0;
#define XB_STRING_ENTRY_FREE(NAME) if(self->NAME) { free(self->NAME); }
#define XB_BOOL_ENTRY_FREE(NAME) self->NAME = 0;
#define XB_DATETIME_ENTRY_FREE(NAME) self->NAME = -1;
#define XB_INT_ENTRY_OPT_FREE(NAME) if(self->NAME ##_set) { self->NAME=0; } self->NAME ## _set = 0;
#define XB_STRING_ENTRY_OPT_FREE(NAME) if(self->NAME) { free(self->NAME); self->NAME=0; }
#define XB_BOOL_ENTRY_OPT_FREE(NAME) if(self->NAME ##_set) { self->NAME=0; } self->NAME ## _set = 0;
#define XB_DATETIME_ENTRY_OPT_FREE(NAME) if(self->NAME ##_set) { self->NAME=-1; } self->NAME ## _set = 0;
#define XB_INT_ENTRY_FIXED_ARRAY_FREE(NAME, COUNT) { int i; for(i=0; i<COUNT; i++) { self->NAME[i] = 0; }}
#define XB_STRING_ENTRY_FIXED_ARRAY_FREE(NAME, COUNT) { int i; for(i=0; i<COUNT; i++) { if(self->NAME[i]) { free(self->NAME[i]); self->NAME[i] = 0; }}}
#define XB_BOOL_ENTRY_FIXED_ARRAY_FREE(NAME, COUNT) { int i; for(i=0; i<COUNT; i++) { self->NAME[i] = 0; }}
#define XB_DATETIME_ENTRY_FIXED_ARRAY_FREE(NAME, COUNT) { int i; for(i=0; i<COUNT; i++) { self->NAME[i] = -1; }}
#define XB_INT_ENTRY_DYNAMIC_ARRAY_FREE(NAME) if(self->NAME) {free(self->NAME); self->NAME ## _count = 0;}
#define XB_STRING_ENTRY_DYNAMIC_ARRAY_FREE(NAME) if(self->NAME) {int i; for(i=0; i<self->NAME ## _count; i++) { free(self->NAME[i]); } free(self->NAME); self->NAME = NULL;}
#define XB_BOOL_ENTRY_DYNAMIC_ARRAY_FREE(NAME) if(self->NAME) {free(self->NAME); self->NAME ## _count = 0;}
#define XB_DATETIME_ENTRY_DYNAMIC_ARRAY_FREE(NAME) if(self->NAME) {free(self->NAME); self->NAME ## _count = 0;}

/**********************************************
 * Unbind Macros                              *
 * These create an XML node tree from C types *
 **********************************************/
#define XB_DEF_UNBIND(STRUCT_TYPE) \
        void STRUCT_TYPE ## _unbind(STRUCT_TYPE *data, xmlNodePtr node);
#define XB_UNBIND_START(STRUCT_TYPE) \
        void STRUCT_TYPE ## _unbind(STRUCT_TYPE *data, xmlNodePtr node) {
#define XB_UNBIND_END }
#define XB_STRUCT_ENTRY_UNBIND(STRUCT_TYPE, XML_NAME, NAME) \
        { xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        STRUCT_TYPE ##  _unbind(&data->NAME, child); xmlAddChild(node, child); }
#define XB_STRUCT_ENTRY_OPT_UNBIND(STRUCT_TYPE, XML_NAME, NAME) \
        if(data->NAME) { xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        STRUCT_TYPE ## _unbind(data->NAME, child); xmlAddChild(node, child); }
#define XB_STRUCT_ENTRY_FIXED_ARRAY_UNBIND(STRUCT_TYPE, XML_NAME, NAME, COUNT) \
        {int i; for(i=0; i<COUNT; i++) { xmlNodePtr child; \
        child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        STRUCT_TYPE ## _unbind(&data->NAME[i], child); xmlAddChild(node, child); }}
#define XB_STRUCT_ENTRY_DYNAMIC_ARRAY_UNBIND(STRUCT_TYPE, XML_NAME, NAME) \
        {int i; for(i=0; i<data->NAME ## _count; i++) { xmlNodePtr child; \
        child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        STRUCT_TYPE ## _unbind(&data->NAME[i], child); xmlAddChild(node, child); }}
#define XB_INT_ENTRY_UNBIND(XML_NAME, NAME) \
        { xmlNodePtr child; char buffer[255]; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        snprintf(buffer, 255, "%lld", data->NAME); \
        xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); }
#define XB_STRING_ENTRY_UNBIND(XML_NAME, NAME) \
        { xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        xmlNodeSetContent(child, data->NAME ? BAD_CAST data->NAME : BAD_CAST ""); \
        xmlAddChild(node, child); }
#define XB_BOOL_ENTRY_UNBIND(XML_NAME, NAME) \
        { xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
         xmlNodeSetContent(child, data->NAME ? BAD_CAST "true" : BAD_CAST "false"); \
         xmlAddChild(node, child); }
#define XB_DATETIME_ENTRY_UNBIND(XML_NAME, NAME) \
        { xmlNodePtr child; char buffer[255]; struct tm a;\
        child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        gmtime_r(&data->NAME, &a); \
        strftime(buffer, 255, "%FT%TZ", &a); \
        xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); }
#define XB_INT_ENTRY_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME ## _set) { xmlNodePtr child; char buffer[255]; \
        child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        snprintf(buffer, 255, "%lld", data->NAME); \
        xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); }
#define XB_STRING_ENTRY_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME) { xmlNodePtr child;\
        child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        xmlNodeSetContent(child, BAD_CAST data->NAME); xmlAddChild(node, child); }
#define XB_BOOL_ENTRY_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME ## _set) { xmlNodePtr child; \
        child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); }
#define XB_DATETIME_ENTRY_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME ## _set) { xmlNodePtr child; char buffer[255]; struct tm a; \
        child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
        gmtime_r(&data->NAME, &a); \
        strftime(buffer, 255, "%FT%TZ", &a); \
        xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); }
#define XB_INT_ENTRY_FIXED_ARRAY_UNBIND(XML_NAME, NAME, COUNT) \
        { int i; for(i=0; i<COUNT; i++) {\
            xmlNodePtr child; char buffer[255]; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            snprintf(buffer, 255, "%lld", data->NAME[i]); \
            xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); } \
        }
#define XB_STRING_ENTRY_FIXED_ARRAY_UNBIND(XML_NAME, NAME, COUNT) \
        { int i; for(i=0; i<COUNT; i++) {\
            xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            xmlNodeSetContent(child, data->NAME[i]); xmlAddChild(node, child); } \
        }
#define XB_BOOL_ENTRY_FIXED_ARRAY_UNBIND(XML_NAME, NAME, COUNT) \
        { int i; for(i=0; i<COUNT; i++) {\
            xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            xmlNodeSetContent(child, data->NAME[i] > BAD_CAST "true", BAD_CAST "false"); \
            xmlAddChild(node, child); } \
        }
#define XB_DATETIME_ENTRY_FIXED_ARRAY_UNBIND(XML_NAME, NAME, COUNT) \
        { int i; for(i=0; i<COUNT; i++) {\
            xmlNodePtr child; char buffer[255]; struct tm a;\
            child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            gmtime_r(&data->NAME[i], &a); \
            strftime(buffer, 255, "%FT%TZ", &a); \
            xmlNodeSetContent(child, buffer); xmlAddChild(node, child); } \
        }
#define XB_INT_ENTRY_DYNAMIC_ARRAY_UNBIND(XML_NAME, NAME) \
        { int i; for(i=0; i<data->NAME ## _count; i++) {\
            xmlNodePtr child; char buffer[255]; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            snprintf(buffer, 255, "%lld", data->NAME[i]); \
            xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); } \
        }
#define XB_STRING_ENTRY_DYNAMIC_ARRAY_UNBIND(XML_NAME, NAME) \
        { int i; for(i=0; i<data->NAME ## _count; i++) {\
            xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            xmlNodeSetContent(child, data->NAME[i] ? BAD_CAST data->NAME[i] : BAD_CAST ""); \
            xmlAddChild(node, child); }\
        }
#define XB_BOOL_ENTRY_DYNAMIC_ARRAY_UNBIND(XML_NAME, NAME) \
        { int i; for(i=0; i<data->NAME ## _count; i++) {\
            xmlNodePtr child; child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            xmlNodeSetContent(child, data->NAME[i] ? BAD_CAST "true" : BAD_CAST "false");\
            xmlAddChild(node, child); } \
        }
#define XB_DATETIME_ENTRY_DYNAMIC_ARRAY_UNBIND(XML_NAME, NAME) \
        { int i; for(i=0; i<data->NAME ## _count; i++) {\
            xmlNodePtr child; char buffer[255]; struct tm a;\
            child = xmlNewNode(NULL, BAD_CAST #XML_NAME); \
            gmtime_r(&data->NAME[i], &a); \
            strftime(buffer, 255, "%FT%TZ", &a); \
            xmlNodeSetContent(child, BAD_CAST buffer); xmlAddChild(node, child); } \
        }
#define XB_INT_ATTR_UNBIND(XML_NAME, NAME) \
        { char buffer[255]; \
        snprintf(buffer, 255, "%lld", data->NAME); \
        xmlNewProp(node, BAD_CAST #XML_NAME, BAD_CAST buffer); }
#define XB_STRING_ATTR_UNBIND(XML_NAME, NAME) \
        xmlNewProp(node, BAD_CAST #XML_NAME, BAD_CAST data->NAME);
#define XB_BOOL_ATTR_UNBIND(XML_NAME, NAME) \
        xmlNewProp(node, BAD_CAST #XML_NAME, data->NAME ? BAD_CAST "true" : BAD_CAST "false");
#define XB_DATETIME_ATTR_UNBIND(XML_NAME, NAME) \
        { char buffer[255]; \
        struct tm a;\
        gmtime_r(&data->NAME, &a); \
        strftime(buffer, 255, "%FT%TZ", &a); \
        xmlNewProp(node, BAD_CAST #XML_NAME, BAD_CAST buffer); }
#define XB_INT_ATTR_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME ## _set) { char buffer[255]; \
        snprintf(buffer, 255, "%lld", data->NAME); \
        xmlNewProp(node, BAD_CAST #XML_NAME, BAD_CAST buffer); }
#define XB_STRING_ATTR_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME ## _set) {\
            xmlNewProp(node, BAD_CAST #XML_NAME, BAD_CAST data->NAME);\
        }
#define XB_BOOL_ATTR_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME ## _set) {\
            xmlNewProp(node, BAD_CAST #XML_NAME, data->NAME ? BAD_CAST "true" : BAD_CAST "false");\
        }
#define XB_DATETIME_ATTR_OPT_UNBIND(XML_NAME, NAME) \
        if(data->NAME ## _set) { char buffer[255]; \
        struct tm a;\
        gmtime_r(&data->NAME, &a); \
        strftime(buffer, 255, "%FT%TZ", &a); \
        xmlNewProp(node, BAD_CAST #XML_NAME, BAD_CAST buffer); }


/******************************************
 * Bind Macros                            *
 * These bind XML node trees into C types *
 ******************************************/
#define XB_DEF_BIND(STRUCT_TYPE) \
        void STRUCT_TYPE ## _bind(STRUCT_TYPE *data, xmlNodePtr node);
#define XB_BIND_START(STRUCT_TYPE) \
        void STRUCT_TYPE ## _bind(STRUCT_TYPE *data, xmlNodePtr node) { \
        xmlNodePtr child; \
        for(child=node->children; child; child=child->next) { \
        if(child->type != XML_ELEMENT_NODE) { \
            continue; }
#define XB_BIND_ITER_END }
#define XB_BIND_END }
#define XB_STRUCT_ENTRY_BIND(STRUCT_TYPE, XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) { \
            STRUCT_TYPE ## _bind(&data->NAME, child); }
#define XB_STRUCT_ENTRY_OPT_BIND(STRUCT_TYPE, XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            data->NAME = STRUCT_TYPE ## _init(malloc(sizeof(STRUCT_TYPE))); \
            STRUCT_TYPE ## _bind(data->NAME, child); }
#define XB_STRUCT_ENTRY_FIXED_ARRAY_BIND(STRUCT_TYPE, XML_NAME, NAME, COUNT) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            STRUCT_TYPE ## _bind(&data->NAME[data->NAME ## _i++], child)};
#define XB_STRUCT_ENTRY_DYNAMIC_ARRAY_BIND(STRUCT_TYPE, XML_NAME, NAME)\
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            STRUCT_TYPE *tmp; \
            tmp = realloc(data->NAME, (data->NAME ## _count + 1) * sizeof(STRUCT_TYPE)); \
            if(!tmp) { fprintf(stderr, "realloc failed"); return; } \
            data->NAME = tmp; STRUCT_TYPE ## _init(&data->NAME[data->NAME ## _count]); \
            STRUCT_TYPE ## _bind(&data->NAME[data->NAME ## _count], child); \
            data->NAME ## _count++; }
#define XB_INT_ENTRY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = strtoll((char*)value, NULL, 10);\
                xmlFree(value); \
            } \
        }
#define XB_STRING_ENTRY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = strdup((char*)value);\
                xmlFree(value); \
            } \
        }
#define XB_BOOL_ENTRY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = strcmp("true", (char*)value) == 0; \
                xmlFree(value); \
            } \
        }
#define XB_DATETIME_ENTRY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = datatime_parse(value);\
                xmlFree(value);\
            } \
        }
#define XB_INT_ENTRY_OPT_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = strtoll((char*)value, NULL, 10);\
                data->NAME ## _set = 1; \
                xmlFree(value); \
            } \
        }
#define XB_STRING_ENTRY_OPT_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = strdup((char*)value);\
                xmlFree(value); \
            } \
        }
#define XB_BOOL_ENTRY_OPT_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = strcmp("true", (char*)value) == 0; \
                data->NAME ## _set = 1; \
                xmlFree(value); \
            } \
        }
#define XB_DATETIME_ENTRY_OPT_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME = datetime_parse(value);\
                data->NAME ## _set = 1; \
                xmlFree(value); \
            } \
        }
#define XB_INT_ENTRY_FIXED_ARRAY_BIND(XML_NAME, NAME, COUNT) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _i++] = strtoll((char*)value, NULL, 10);\
                xmlFree(value); \
            } \
        }
#define XB_STRING_ENTRY_FIXED_ARRAY_BIND(XML_NAME, NAME, COUNT) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _i++] = strdup((char*)value);\
                xmlFree(value); \
            } \
        }

#define XB_BOOL_ENTRY_FIXED_ARRAY_BIND(XML_NAME, NAME, COUNT) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _i++] = strcmp("true", (char*)value) == 0; \
                xmlFree(value); \
            } \
        }

#define XB_DATETIME_ENTRY_FIXED_ARRAY_BIND(XML_NAME, NAME, COUNT) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _i++] = datetime_parse(value);\
                xmlFree(value); \
            } \
        }
#define XB_INT_ENTRY_DYNAMIC_ARRAY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            int *tmp; \
            tmp = realloc(data->NAME, (data->NAME ## _count + 1) * sizeof(long long)); \
            if(!tmp) { fprintf(stderr, "realloc failed"); return; } \
            data->NAME = tmp; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _count++] = strtoll((char*)value, NULL, 10);\
                xmlFree(value); \
            } \
        }
#define XB_STRING_ENTRY_DYNAMIC_ARRAY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            char **tmp; \
            tmp = realloc(data->NAME, (data->NAME ## _count + 1) * sizeof(char*)); \
            if(!tmp) { fprintf(stderr, "realloc failed"); return; } \
            data->NAME = tmp; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _count++] = strdup((char*)value);\
                xmlFree(value); \
            } \
        }
#define XB_BOOL_ENTRY_DYNAMIC_ARRAY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            int *tmp; \
            tmp = realloc(data->NAME, (data->NAME ## _count + 1) * sizeof(int)); \
            if(!tmp) { fprintf(stderr, "realloc failed"); return; } \
            data->NAME = tmp; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _i++] = strcmp("true", (char*)value) == 0; \
                xmlFree(value); \
            } \
        }
#define XB_DATETIME_ENTRY_DYNAMIC_ARRAY_BIND(XML_NAME, NAME) \
        else if(!strcmp(#XML_NAME, (char*)child->name)) {\
            xmlChar *value; \
            time_t *tmp; \
            tmp = realloc(data->NAME, (data->NAME ## _count + 1) * sizeof(time_t)); \
            if(!tmp) { fprintf(stderr, "realloc failed"); return; } \
            data->NAME = tmp; \
            value = xmlNodeGetContent(child); \
            if(value) { \
                data->NAME[data->NAME ## _count++] = datetime_parse(value);\
                xmlFree(value); \
            } \
        }
#define XB_INT_ATTR_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        data->NAME = strtoll((char*)value, NULL, 10);\
        xmlFree(value); \
    }}
#define XB_STRING_ATTR_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        data->NAME = strdup((char*)value);\
        xmlFree(value); \
    }}
#define XB_BOOL_ATTR_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        data->NAME = strcmp("true", (char*)value) == 0; \
        xmlFree(value); \
    }}
#define XB_DATETIME_ATTR_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        struct tm t; \
        t.tm_gmtoff=0; \
        char buffer[255]; \
        /* strptime doesn't accept "Z" as GMT.*/\
        strlcpy(buffer, (char*)value, 255); \
        buffer[strlen(buffer)-1] = 0;\
        strlcat(buffer, "GMT", 255);\
        if(!strptime(buffer, "%FT%T%Z", &t)) { \
            fprintf(stderr, "Could not parse dateTime value %s\n", buffer); \
            return; \
        }\
        data->NAME = mktime(&t);\
        xmlFree(value); \
    }}
#define XB_INT_ATTR_OPT_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        data->NAME = strtoll((char*)value, NULL, 10);\
        data->NAME ## _set = 1; \
        xmlFree(value); \
    }}
#define XB_STRING_ATTR_OPT_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        data->NAME = strdup((char*)value);\
        xmlFree(value); \
    }}
#define XB_BOOL_ATTR_OPT_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        data->NAME = strcmp("true", (char*)value) == 0; \
        data->NAME ## _set = 1; \
        xmlFree(value); \
    }}
#define XB_DATETIME_ATTR_OPT_BIND(XML_NAME, NAME) \
{ xmlChar *value = NULL; \
    value = xmlGetProp(node, BAD_CAST #XML_NAME); \
    if(value) {\
        struct tm t; \
        t.tm_gmtoff=0; \
        char buffer[255]; \
        /* strptime doesn't accept "Z" as GMT.*/\
        strlcpy(buffer, (char*)value, 255); \
        buffer[strlen(buffer)-1] = 0;\
        strlcat(buffer, "GMT", 255);\
        if(!strptime(buffer, "%FT%T%Z", &t)) { \
            fprintf(stderr, "Could not parse dateTime value %s\n", buffer); \
            return; \
        }\
        data->NAME = mktime(&t);\
        data->NAME ## _set = 1; \
        xmlFree(value); \
    }}


#endif /* XMLBIND_MACROS_H_ */
