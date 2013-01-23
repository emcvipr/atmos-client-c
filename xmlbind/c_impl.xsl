<?xml version="1.0" encoding="UTF-8"?>
<!--
 Copyright (c) 2013, EMC Corporation

 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name of the EMC Corporation nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 -->
 
<!-- Binder implementation for C.  Separated into another file so it can be  
     included into other stylesheets (i.e. it doesn't have a "/" template) -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"	
	xmlns:exsl="http://exslt.org/common" 
	xmlns:xsd="http://www.w3.org/2001/XMLSchema"
	xmlns:str="http://exslt.org/strings" 
	xmlns:set="http://exslt.org/sets"
	extension-element-prefixes="str exsl set">
	
	<!-- Main template.  Requires the $p-module-name parameter that defines
	     the name of the output files.  Also uses the global parameter 
	     $prefix to prefix the C type and function names. -->
	<xsl:template name="xsd-to-c">
		<xsl:param name="p-module-name"/>
		<xsl:variable name="hfile" select="concat($p-module-name, '.h')"/>
		<xsl:variable name="cfile" select="concat($p-module-name, '.c')"/>
		
		<!-- ********************* 
		     * Build the .h file *
		     ********************* -->
		<exsl:document href="{$hfile}" method="text" omit-xml-declaration="yes">
			<!-- H Header Preamble -->
			<xsl:text>#ifndef </xsl:text>
			<xsl:call-template name="uppercase">
				<xsl:with-param name="value" select="$p-module-name"/>
			</xsl:call-template>
			<xsl:text>_H</xsl:text>
			<xsl:call-template name="newline"/>
			<xsl:text>#define </xsl:text>
			<xsl:call-template name="uppercase">
				<xsl:with-param name="value" select="$p-module-name"/>
			</xsl:call-template>
			<xsl:text>_H</xsl:text>
			<xsl:call-template name="newline"/>
			<xsl:text>
#include &lt;string.h&gt;
#include &lt;time.h&gt;
#include &lt;stdint.h&gt;
#include &lt;libxml/tree.h&gt;
#include &lt;libxml/parser.h&gt;

</xsl:text>
			
			<!-- Define Complex Types -->
			<xsl:call-template name="process_complex_types">
				<xsl:with-param name="types_to_process" select="/xsd:schema/xsd:complexType"/>
				<xsl:with-param name="root" select="/"/>
			</xsl:call-template>
			
			<!-- Add marshallers for toplevel elements -->
			<xsl:for-each select="/xsd:schema/xsd:element">
				<xsl:text>/* Toplevel element </xsl:text>
				<xsl:value-of select="@name"/>
				<xsl:text> */</xsl:text>
				<xsl:call-template name="newline"/>
				<xsl:call-template name="declare_marshaller"/>
				<xsl:call-template name="newline"/>
				<xsl:call-template name="declare_unmarshaller"/>
				<xsl:call-template name="newline"/>
			</xsl:for-each>
			
			<xsl:call-template name="newline"/>
			<xsl:text>#endif</xsl:text>
			<xsl:call-template name="newline"/>
		</exsl:document>
		
		<!-- ********************* 
		     * Build the .c file *
		     ********************* -->
		<exsl:document href="{$cfile}" method="text" omit-xml-declaration="yes">
		
			<!-- C File Preamble -->
			<xsl:text>#include &lt;string.h&gt;
#include &lt;time.h&gt;
#include &lt;stdint.h&gt;
#include &lt;inttypes.h&gt;
#include &lt;libxml/tree.h&gt;
#include &lt;libxml/parser.h&gt;

#include &quot;</xsl:text>
			<xsl:value-of select="$hfile"/><xsl:text>&quot;</xsl:text>
			<xsl:call-template name="newline"/>
			<xsl:call-template name="newline"/>
			
			<!-- Include helper functions -->
			<xsl:call-template name="c-helper-functions"/>
			
			<!-- Define Complex Type functions -->
			<xsl:for-each select="/xsd:schema/xsd:complexType">
				<xsl:text>/* Functions for </xsl:text>
				<xsl:call-template name="ct_to_struct_name">
					<xsl:with-param name="type-name" select="@name"/>
				</xsl:call-template>
				<xsl:text> */</xsl:text>
				<xsl:call-template name="newline"/>
				<xsl:call-template name="implement-constructor">
					<xsl:with-param name="type" select="."/>
				</xsl:call-template>
				<xsl:call-template name="implement-destructor">
					<xsl:with-param name="type" select="."/>
				</xsl:call-template>
				<xsl:call-template name="implement-binder">
					<xsl:with-param name="type" select="."/>
				</xsl:call-template>
				<xsl:call-template name="implement-unbinder">
					<xsl:with-param name="type" select="."/>
				</xsl:call-template>
				<xsl:call-template name="newline"/>
			</xsl:for-each>
			
			<!-- Add marshallers for toplevel elements -->
			<xsl:for-each select="/xsd:schema/xsd:element">
				<xsl:text>/* Toplevel element </xsl:text>
				<xsl:value-of select="@name"/>
				<xsl:text> */</xsl:text>
				
 				<xsl:call-template name="newline"/>
				<xsl:call-template name="implement_marshaller"/>
				<xsl:call-template name="newline"/>
				<xsl:call-template name="implement_unmarshaller"/>
				<xsl:call-template name="newline"/>
			</xsl:for-each>
		</exsl:document>
		
	</xsl:template>
	
	<!-- Recursive template to process Complex Types -->
	<xsl:template name="process_complex_types">
		<xsl:param name="types_to_process" select="/.."/>
		<xsl:param name="processed_types" select="/.."/>
		<!-- We pass in an explicit reference to the source document root.   -->
		<!-- Some of the templates are called with node sets derived from    -->
		<!-- exsl:node-set(), and lose their link to the original document   -->
		<!-- so you can't do relative selects against them and get the same  -->
		<!-- results as if done against the real source document.            -->
		<xsl:param name="root"/>
		
		<xsl:variable name="x" select="$types_to_process"/>
		
		<xsl:if test="$types_to_process[1]">
			<xsl:variable name="type" select="$types_to_process[1]"/>
			
			<!-- Make sure it hasn't been processed yet -->
			<xsl:choose>
			<xsl:when test="not($processed_types[@name=$type/@name])">
				<!-- Compute the set of child types -->
				<xsl:variable name="child_type_names" select="$type/xsd:sequence/xsd:element/@type"/>
				<xsl:variable name="child_types">
				<xsl:if test="count($child_type_names) &gt; 0">
				<xsl:call-template name="ct_names_to_types">
						<xsl:with-param name="type_names" select="$child_type_names"/>
						<xsl:with-param name="processed_types" select="$processed_types"/>
						<xsl:with-param name="root" select="$root"/>
					</xsl:call-template>
				</xsl:if>
				</xsl:variable>
				
				<xsl:choose>
					<xsl:when test="exsl:node-set($child_types)/xsd:complexType">
						<!-- Process the child types first.  Append this type to -->
						<!-- the end and try again later. -->
						
						<xsl:message>
							<xsl:value-of select="concat('TYPE HAS UNSATISFIED DEPS: ', count(exsl:node-set($child_types)/xsd:complexType))"/>
						</xsl:message>
						
						<!-- Note: can't use the XSL union operator here     -->
						<!-- because it will return the elements in document -->
						<!-- order and that's not what we want.              -->
						<xsl:variable name="new_types_to_process">
							<xsl:copy-of select="$types_to_process[position() != 1]"/>
							<xsl:copy-of select="$type"/>
						</xsl:variable>
						
						<xsl:call-template name="process_complex_types">
							<xsl:with-param name="types_to_process" select="exsl:node-set($new_types_to_process)/xsd:complexType"/>
							<xsl:with-param name="processed_types" select="$processed_types"/>
							<xsl:with-param name="root" select="$root"/>
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>
						<!-- Dependencies satisfied.  Process this type. -->
						<xsl:text>/* Complex type </xsl:text>
						<xsl:value-of select="$type/@name"/>
						<xsl:text> */</xsl:text>
						<xsl:call-template name="newline"/>
						<xsl:call-template name="newline"/>
						
						<xsl:call-template name="start-struct">
							<xsl:with-param name="type" select="$type"/>
						</xsl:call-template>
						<xsl:call-template name="build-struct">
							<xsl:with-param name="type" select="$type"/>
						</xsl:call-template>
						<xsl:call-template name="end-struct">
							<xsl:with-param name="type" select="$type"/>
						</xsl:call-template>
						
						<xsl:call-template name="newline"/>
						<xsl:call-template name="newline"/>
						
						<xsl:call-template name="declare-constructor">
							<xsl:with-param name="type" select="$type"/>
						</xsl:call-template>
						<xsl:call-template name="declare-destructor">
							<xsl:with-param name="type" select="$type"/>
						</xsl:call-template>
						<xsl:call-template name="declare-binder">
							<xsl:with-param name="type" select="$type"/>
						</xsl:call-template>
						<xsl:call-template name="declare-unbinder">
							<xsl:with-param name="type" select="$type"/>
						</xsl:call-template>
						<!-- Recurse to next type -->
						<xsl:call-template name="process_complex_types">
							<xsl:with-param name="types_to_process" select="$types_to_process[position() != 1]"/>
							<xsl:with-param name="processed_types" select="$processed_types|$type"/>
							<xsl:with-param name="root" select="$root"/>
						</xsl:call-template>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<!-- This type has already been processed.  Skip it and continue -->
				<xsl:call-template name="process_complex_types">
					<xsl:with-param name="types_to_process" select="$types_to_process[position() != 1]"/>
					<xsl:with-param name="processed_types" select="$processed_types"/>
					<xsl:with-param name="root" select="$root"/>
				</xsl:call-template>
			</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:template>
	
	<!-- Starts the struct for a type -->
	<xsl:template name="start-struct">
		<xsl:param name="type"/>
		<xsl:text>/* Structure to encapsulate </xsl:text>
		<xsl:value-of select="$type/@name"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>typedef struct {</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	<!-- Builds the struct for a type -->
	<xsl:template name="build-struct">
		<xsl:param name="type"/>
		<!-- Sequence Elements -->
		<xsl:for-each select="$type/xsd:sequence/xsd:element">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@maxOccurs = 'unbounded' or @maxOccurs > 1">
						<xsl:value-of select="string('array')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 1 and @maxOccurs = 1">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0 and @maxOccurs = 1">
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0">
						<!-- default per spec max = 1 -->
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@maxOccurs = 1">
						<!-- default per spec min = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- No min/max -->
						<!-- default per spec is min = 1 and max = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:call-template name="xsd_type_to_c_struct">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
			</xsl:call-template>
		</xsl:for-each>
		
		<!-- Attributes -->
		<xsl:for-each select="$type/xsd:attribute">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@required = 'true'">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="string('optional')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:call-template name="xsd_type_to_c_struct">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
			</xsl:call-template>
		</xsl:for-each>
	</xsl:template>
	
	<!-- Ends the struct for a type -->
	<xsl:template name="end-struct">
		<xsl:param name="type"/>
		<xsl:text>} </xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>;</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>		
	</xsl:template>
	
	<!-- Declares constructor for a type -->
	<xsl:template name="declare-constructor">
		<xsl:param name="type"/>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_init(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self);</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>		
	</xsl:template>
	
	<!-- Declares destructor for a type -->
	<xsl:template name="declare-destructor">
		<xsl:param name="type"/>
		<xsl:text>void </xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_destroy(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self);</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>		
	</xsl:template>
	
	<!-- Declares binder for a type -->
	<xsl:template name="declare-binder">
		<xsl:param name="type"/>
		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Binds the XML data in the node parameter to the struct</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>void </xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_bind(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self, xmlNodePtr node);</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>		
	</xsl:template>

	<!-- Declares unbinder for a type -->
	<xsl:template name="declare-unbinder">
		<xsl:param name="type"/>
		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Unbinds the struct, populating the given XML node</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>void </xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_unbind(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self, xmlDocPtr doc, xmlNodePtr node);</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>		
	</xsl:template>
	
	<!-- Converts an XSD type (element or attribute) to a C struct member -->
	<xsl:template name="xsd_type_to_c_struct">
		<xsl:param name="type"/> <!-- Either an xsd:element or an xsd:attribute -->
		<xsl:param name="cardinality"/>
		<xsl:variable name="type-lookup">
			<xsl:call-template name="xsd_type_reference_to_qname">
				<xsl:with-param name="type-name" select="$type/@type"/>
			</xsl:call-template>
		</xsl:variable>
		<!-- Convert it into a nodeset so we can select against it -->
		<xsl:variable name="type-lookup-nodeset" select="exsl:node-set($type-lookup)"/>
		<xsl:variable name="type-namespace" select="string($type-lookup-nodeset/namespace)"/>
		<xsl:choose>
			<!-- Need to check the namespace instead of the type passed in   -->
			<!-- because we can't be sure they're using the xsd: prefix      -->
			<!-- (sometimes xs: or even s: is used).                         -->
			<xsl:when test="$type-namespace = 'http://www.w3.org/2001/XMLSchema'">
				<!-- It's a builtin type -->
				<xsl:choose>
					<xsl:when test="$type-lookup-nodeset/localName = 'string'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required' or $cardinality = 'optional'">
								<!-- Single string, null if optional and unset -->
								<xsl:text>    char *</xsl:text>
								<xsl:call-template name="element_to_c_name">
									<xsl:with-param name="element-name" select="$type/@name"/>
								</xsl:call-template>
								<xsl:text>;</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of strings -->
								<xsl:text>    char **</xsl:text>
								<xsl:call-template name="element_to_c_name">
									<xsl:with-param name="element-name" select="$type/@name"/>
								</xsl:call-template>
								<xsl:text>;</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>    int </xsl:text>
								<xsl:call-template name="element_to_c_name">
									<xsl:with-param name="element-name" select="$type/@name"/>
								</xsl:call-template>
								<xsl:text>_count;</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'int'">
						<xsl:call-template name="simple_type_to_struct">
							<xsl:with-param name="xsd_type" select="$type"/>
							<xsl:with-param name="c_type" select="'int64_t'"/>
							<xsl:with-param name="cardinality" select="$cardinality"/>
						</xsl:call-template>
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'dateTime'">
						<xsl:call-template name="simple_type_to_struct">
							<xsl:with-param name="xsd_type" select="$type"/>
							<xsl:with-param name="c_type" select="'time_t'"/>
							<xsl:with-param name="cardinality" select="$cardinality"/>
						</xsl:call-template>
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'boolean'">
						<xsl:call-template name="simple_type_to_struct">
							<xsl:with-param name="xsd_type" select="$type"/>
							<xsl:with-param name="c_type" select="'int'"/>
							<xsl:with-param name="cardinality" select="$cardinality"/>
						</xsl:call-template>
					</xsl:when>
					<xsl:otherwise>
						<xsl:message terminate="yes">
							<xsl:text>Unsupported builtin type </xsl:text>
							<xsl:value-of select="string($type)"/>
						</xsl:message>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<!-- It's a complex type -->
				<xsl:choose>
					<xsl:when test="$cardinality = 'required'">
						<!-- Required, embedded struct -->
						<xsl:text>    </xsl:text>
						<xsl:call-template name="ct_to_struct_name">
							<xsl:with-param name="type-name" select="$type/@type"></xsl:with-param>
						</xsl:call-template>
						<xsl:text> </xsl:text>
						<xsl:call-template name="element_to_c_name">
							<xsl:with-param name="element-name" select="$type/@name"/>
						</xsl:call-template>
						<xsl:text>;</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:when test="$cardinality = 'optional'">
						<!-- Optional, struct pointer -->
						<xsl:text>    </xsl:text>
						<xsl:call-template name="ct_to_struct_name">
							<xsl:with-param name="type-name" select="$type/@type"></xsl:with-param>
						</xsl:call-template>
						<xsl:text> *</xsl:text>
						<xsl:call-template name="element_to_c_name">
							<xsl:with-param name="element-name" select="$type/@name"/>
						</xsl:call-template>
						<xsl:text>;</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- Complex type array -->
						<xsl:text>    </xsl:text>
						<xsl:call-template name="ct_to_struct_name">
							<xsl:with-param name="type-name" select="$type/@type"/>
						</xsl:call-template>
						<xsl:text> *</xsl:text>
						<xsl:call-template name="element_to_c_name">
							<xsl:with-param name="element-name" select="$type/@name"/>
						</xsl:call-template>
						<xsl:text>;</xsl:text>
						<xsl:call-template name="newline"/>
						<xsl:text>    int </xsl:text>
						<xsl:call-template name="element_to_c_name">
							<xsl:with-param name="element-name" select="$type/@name"/>
						</xsl:call-template>
						<xsl:text>_count;</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<!-- Converts a simple type (excl xsd:string) to a C struct member -->
	<xsl:template name="simple_type_to_struct">
		<xsl:param name="xsd_type"/><!-- xsd:element or xsd:attribute -->
		<xsl:param name="c_type"/>
		<xsl:param name="cardinality"/>
		<xsl:choose>
			<xsl:when test="$cardinality = 'required'">
				<xsl:text>    </xsl:text>
				<xsl:value-of select="string($c_type)"/>
				<xsl:text> </xsl:text>
				<xsl:call-template name="element_to_c_name">
					<xsl:with-param name="element-name" select="$xsd_type/@name"/>
				</xsl:call-template>
				<xsl:text>;</xsl:text>
				<xsl:call-template name="newline"/>
			</xsl:when>
			<xsl:when test="$cardinality = 'optional'">
				<!-- Optional member, add a 2nd member for _set -->
				<xsl:text>    </xsl:text>
				<xsl:value-of select="string($c_type)"/>
				<xsl:text> </xsl:text>
				<xsl:call-template name="element_to_c_name">
					<xsl:with-param name="element-name" select="$xsd_type/@name"/>
				</xsl:call-template>
				<xsl:text>;</xsl:text>
				<xsl:call-template name="newline"/>
				<xsl:text>    int </xsl:text>
				<xsl:call-template name="element_to_c_name">
					<xsl:with-param name="element-name" select="$xsd_type/@name"/>
				</xsl:call-template>
				<xsl:text>_set;</xsl:text>
				<xsl:call-template name="newline"/>
			</xsl:when>
			<xsl:otherwise>
				<!-- Array type -->
				<xsl:text>    </xsl:text>
				<xsl:value-of select="string($c_type)"/>
				<xsl:text> *</xsl:text>
				<xsl:call-template name="element_to_c_name">
					<xsl:with-param name="element-name" select="$xsd_type/@name"/>
				</xsl:call-template>
				<xsl:text>;</xsl:text>
				<xsl:call-template name="newline"/>
				<xsl:text>    int </xsl:text>
				<xsl:call-template name="element_to_c_name">
					<xsl:with-param name="element-name" select="$xsd_type/@name"/>
				</xsl:call-template>
				<xsl:text>_count;</xsl:text>
				<xsl:call-template name="newline"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<!-- Template to declare the marshaller -->
	<xsl:template name="declare_marshaller">
		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Marshals a </xsl:text>
		<xsl:call-template name="ct_to_struct_name"/>
		<xsl:text> structure to an XML document containing a &lt;</xsl:text>
		<xsl:value-of select="@name"></xsl:value-of>
		<xsl:text>&gt; element.</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>

		<xsl:text>xmlChar *</xsl:text>
		<xsl:call-template name="ct_to_struct_name"/>
		<xsl:text>_marshal(</xsl:text>
		<xsl:call-template name="ct_to_struct_name"/>
		<xsl:text> *self);</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	<!-- Template to declare the unmarshaller -->
	<xsl:template name="declare_unmarshaller">
		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Unmarshals XML containing a &lt;</xsl:text>
		<xsl:value-of select="@name"></xsl:value-of>
		<xsl:text>&gt; element to a </xsl:text>
		<xsl:call-template name="ct_to_struct_name"/>
		<xsl:text> structure.</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>

		<xsl:call-template name="ct_to_struct_name"/>
		<xsl:text> *</xsl:text>
		<xsl:call-template name="ct_to_struct_name"/>
		<xsl:text>_unmarshal(const char *xml);</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	<!-- Template to convert an element or attribute name into a C name -->
	<xsl:template name="element_to_c_name">
		<xsl:param name="element-name" select="string(@name)"/>
		<!-- If namespaces are involved, extract only the local part -->
		<xsl:variable name="local-type-name">
			<xsl:choose>
				<xsl:when test="contains($element-name, ':')">
					<xsl:value-of select="substring-after($element-name, ':')"></xsl:value-of>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="string($element-name)"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		
		<!-- Hyphens go to underscores -->
		<!-- Also, while uncommon, periods are acceptable and also translated to underscores -->
		<xsl:value-of select="translate($local-type-name, '-.', '__')"/>
	</xsl:template>
	
	<!-- Template to convert a complex type name into a struct name -->
	<xsl:template name="ct_to_struct_name">
		<xsl:param name="type-name" select="string(@type)"/>
		
		<!-- If namespaces are involved, extract only the local part -->
		<xsl:variable name="local-type-name">
			<xsl:choose>
				<xsl:when test="contains($type-name, ':')">
					<xsl:value-of select="substring-after($type-name, ':')"></xsl:value-of>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="string($type-name)"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:if test="$prefix"><xsl:value-of select="$prefix"/>_</xsl:if>
		<xsl:variable name="hyphensplit">
			<xsl:call-template name="token_list_to_camel_case">
				<xsl:with-param name="tokens" select="str:split($local-type-name, '-')"/>
			</xsl:call-template>
		</xsl:variable>
		<xsl:variable name="underscoresplit">
			<xsl:call-template name="token_list_to_camel_case">
				<xsl:with-param name="tokens" select="str:split(string($hyphensplit), '_')"/>
			</xsl:call-template>
		</xsl:variable>
		<xsl:value-of select="string($underscoresplit)"/>
	</xsl:template>
	
		<!-- Template to implement the marshaller -->
	<xsl:template name="implement_marshaller">
		<xsl:variable name="c-type">
			<xsl:call-template name="ct_to_struct_name"/>
		</xsl:variable>
		
		<!-- Get the full XML name for the element -->
		<xsl:variable name="xml-name-fragment">
			<xsl:call-template name="xsd_name_reference_to_qname">
				<xsl:with-param name="type-name" select="./@name"/>
			</xsl:call-template>
		</xsl:variable>
		<!-- Convert it into a nodeset so we can select against it -->
		<xsl:variable name="xml-name-set" select="exsl:node-set($xml-name-fragment)"/>
	
		<xsl:variable name="xml-name" select="$xml-name-set/localName"/>
		<xsl:variable name="xml-namespace-prefix" select="$xml-name-set/prefix"/>
		<xsl:variable name="xml-namespace" select="$xml-name-set/namespace"/>

		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Marshals a </xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text> structure to an XML document containing a &lt;</xsl:text>
		<xsl:value-of select="@name"></xsl:value-of>
		<xsl:text>&gt; element.</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>xmlChar *</xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text>_marshal(</xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text> *self) {</xsl:text>
		<xsl:call-template name="newline"/>
		
		<xsl:text>    xmlNodePtr n;</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    xmlDocPtr doc;</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    xmlChar *xmlbuff;</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    int buffersize;</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>
		
		<xsl:choose>
			<xsl:when test="$xml-namespace = ''">
				<!-- No namespace on root node. -->
				<xsl:text>    doc = xmlNewDoc(BAD_CAST "1.0");</xsl:text><xsl:call-template name="newline"/>
				<xsl:text>    n = xmlNewNode(NULL, BAD_CAST &quot;</xsl:text>
				<xsl:value-of select="$xml-name"/>
				<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
			</xsl:when>
			<xsl:otherwise>
				<!-- Need a namespace for the root node -->
				<xsl:text>    xmlNsPtr ns;</xsl:text><xsl:call-template name="newline"/>
				<xsl:text>    doc = xmlNewDoc(BAD_CAST "1.0");</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:text>    n = xmlNewNode(NULL, BAD_CAST &quot;</xsl:text>
				<xsl:value-of select="$xml-name"/>
				<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:text>    ns = xmlNewNs(n, BAD_CAST &quot;</xsl:text>
				<xsl:value-of select="$xml-namespace"/>
				<xsl:text>&quot;, BAD_CAST &quot;</xsl:text>
				<xsl:value-of select="$xml-name"/>
				<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:text>    xmlSetNs(n, ns);</xsl:text><xsl:call-template name="newline"/>
			</xsl:otherwise>
		</xsl:choose>
		
		<xsl:text>    xmlDocSetRootElement(doc, n);</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>    </xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text>_unbind(self, doc, n);</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>    xmlDocDumpFormatMemoryEnc(doc, &amp;xmlbuff, &amp;buffersize, "UTF-8", 1);</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    xmlFreeDoc(doc);</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>
		<xsl:text>    return(xmlbuff);</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>}</xsl:text><xsl:call-template name="newline"/>
	</xsl:template>
	
	<!-- Template to declare the unmarshaller -->
	<xsl:template name="implement_unmarshaller">
		<xsl:variable name="c-type">
			<xsl:call-template name="ct_to_struct_name"/>
		</xsl:variable>
	
		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Unmarshals XML containing a &lt;</xsl:text>
		<xsl:value-of select="@name"></xsl:value-of>
		<xsl:text>&gt; element to a </xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text> structure.</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>

		<xsl:value-of select="$c-type"/>
		<xsl:text> *</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:value-of select="$c-type"/>
		<xsl:text>_unmarshal(const char *xml) {</xsl:text>
		<xsl:call-template name="newline"/>
		
		<xsl:text>    </xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text>* data;</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>    xmlDocPtr doc;</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    xmlNodePtr root;</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    int docsz;</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>
		
		<xsl:text>    data = </xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text>_init(malloc(sizeof(</xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text>)));</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>    docsz = (int)strlen(xml);</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    doc = xmlReadMemory(xml, docsz, "noname.xml", NULL, 0);</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    if(doc == NULL) {</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>        </xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text>_destroy(data);</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>        free(data);</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>        return NULL;</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
		<xsl:call-template name="newline"/>
		<xsl:text>    root = xmlDocGetRootElement(doc);</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>    </xsl:text>
		<xsl:value-of select="$c-type"/>
		<xsl:text>_bind(data, root);</xsl:text><xsl:call-template name="newline"/>
		
		<xsl:text>    xmlFreeDoc(doc);</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>    return data;</xsl:text><xsl:call-template name="newline"/>
		<xsl:text>}</xsl:text><xsl:call-template name="newline"/>
	</xsl:template>
	
		<!-- Declares constructor for a type -->
	<xsl:template name="implement-constructor">
		<xsl:param name="type"/>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_init(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self) {</xsl:text>
		<xsl:call-template name="newline"/>
		
		<xsl:text>    memset(self, 0, sizeof(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>));</xsl:text>
		<xsl:call-template name="newline"/>
		
		<xsl:text>    return self;</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>}</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	<!-- Declares destructor for a type -->
	<xsl:template name="implement-destructor">
		<xsl:param name="type"/>
		<xsl:text>void </xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_destroy(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self) {</xsl:text>
		<xsl:call-template name="newline"/>
		
		<!-- Sequence Elements -->
		<xsl:for-each select="$type/xsd:sequence/xsd:element">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@maxOccurs = 'unbounded' or @maxOccurs > 1">
						<xsl:value-of select="string('array')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 1 and @maxOccurs = 1">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0 and @maxOccurs = 1">
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0">
						<!-- default per spec max = 1 -->
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@maxOccurs = 1">
						<!-- default per spec min = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- No min/max -->
						<!-- default per spec is min = 1 and max = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:call-template name="xsd_type_destroy">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
			</xsl:call-template>
		</xsl:for-each>
		
		<!-- Attributes -->
		<xsl:for-each select="$type/xsd:attribute">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@required = 'true'">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="string('optional')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:call-template name="xsd_type_destroy">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
			</xsl:call-template>
		</xsl:for-each>
		
		<xsl:call-template name="newline"/>
		<xsl:text>}</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	<!-- Declares binder for a type -->
	<xsl:template name="implement-binder">
		<xsl:param name="type"/>
		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Binds the XML data in the node parameter to the struct</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>void </xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_bind(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self, xmlNodePtr node) {</xsl:text>
		<xsl:call-template name="newline"/>
		
		<xsl:text>    xmlNodePtr child;

    for(child=node-&gt;children; child; child=child-&gt;next) {
        if(child-&gt;type != XML_ELEMENT_NODE) {
            continue;</xsl:text>
		<xsl:call-template name="newline"/>
		
		<!-- Sequence Elements -->
		<xsl:for-each select="$type/xsd:sequence/xsd:element">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@maxOccurs = 'unbounded' or @maxOccurs > 1">
						<xsl:value-of select="string('array')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 1 and @maxOccurs = 1">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0 and @maxOccurs = 1">
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0">
						<!-- default per spec max = 1 -->
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@maxOccurs = 1">
						<!-- default per spec min = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- No min/max -->
						<!-- default per spec is min = 1 and max = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:call-template name="xsd_type_bind">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
				<xsl:with-param name="is-attribute" select="'false'"/>
			</xsl:call-template>
		</xsl:for-each>
		
		<xsl:call-template name="newline"/>
		<xsl:text>        }</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>    }</xsl:text>
		<xsl:call-template name="newline"/>

		<!-- Attributes -->
		<xsl:for-each select="$type/xsd:attribute">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@required = 'true'">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="string('optional')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:call-template name="xsd_type_bind">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
				<xsl:with-param name="is-attribute" select="'true'"/>
			</xsl:call-template>
		</xsl:for-each>
		
		<xsl:call-template name="newline"/>
		<xsl:text>}</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>

	<!-- Declares unbinder for a type -->
	<xsl:template name="implement-unbinder">
		<xsl:param name="type"/>
		<xsl:text>/**</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> * Unbinds the struct, populating the given XML node</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text> */</xsl:text>
		<xsl:call-template name="newline"/>
		<xsl:text>void </xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text>_unbind(</xsl:text>
		<xsl:call-template name="ct_to_struct_name">
			<xsl:with-param name="type-name" select="$type/@name"/>
		</xsl:call-template>
		<xsl:text> *self, xmlDocPtr doc, xmlNodePtr node) {</xsl:text>
		<xsl:call-template name="newline"/>
		
		<!-- Sequence Elements -->
		<xsl:for-each select="$type/xsd:sequence/xsd:element">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@maxOccurs = 'unbounded' or @maxOccurs > 1">
						<xsl:value-of select="string('array')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 1 and @maxOccurs = 1">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0 and @maxOccurs = 1">
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@minOccurs = 0">
						<!-- default per spec max = 1 -->
						<xsl:value-of select="string('optional')"/>
					</xsl:when>
					<xsl:when test="@maxOccurs = 1">
						<!-- default per spec min = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- No min/max -->
						<!-- default per spec is min = 1 and max = 1 -->
						<xsl:value-of select="string('required')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:call-template name="xsd_type_unbind">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
				<xsl:with-param name="is-attribute" select="'false'"/>
			</xsl:call-template>
		</xsl:for-each>
		
		<!-- Attributes -->
		<xsl:for-each select="$type/xsd:attribute">
			<xsl:variable name="cardinality">
				<xsl:choose>
					<xsl:when test="@use = 'required'">
						<xsl:value-of select="string('required')"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="string('optional')"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:variable>
			<xsl:text>    {</xsl:text>
			<xsl:call-template name="newline"/>
			<xsl:call-template name="xsd_type_unbind">
				<xsl:with-param name="type" select="."/>
				<xsl:with-param name="cardinality" select="$cardinality"/>
				<xsl:with-param name="is-attribute" select="'true'"/>
			</xsl:call-template>
			<xsl:text>    }</xsl:text>
			<xsl:call-template name="newline"/>
		</xsl:for-each>
		
		<xsl:call-template name="newline"/>
		<xsl:text>}</xsl:text>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	<!-- Handles freeing the struct members for each element -->
	<xsl:template name="xsd_type_destroy">
		<xsl:param name="type"/> <!-- Either an xsd:element or an xsd:attribute -->
		<xsl:param name="cardinality"/>
		<xsl:variable name="type-lookup">
			<xsl:call-template name="xsd_type_reference_to_qname">
				<xsl:with-param name="type-name" select="$type/@type"/>
			</xsl:call-template>
		</xsl:variable>
		<!-- Convert it into a nodeset so we can select against it -->
		<xsl:variable name="type-lookup-nodeset" select="exsl:node-set($type-lookup)"/>
		<xsl:variable name="type-namespace" select="string($type-lookup-nodeset/namespace)"/>
		
		<!-- Get the C name for the element -->
		<xsl:variable name="c-name">
			<xsl:call-template name="element_to_c_name">
				<xsl:with-param name="element-name" select="$type/@name"/>
			</xsl:call-template>
		</xsl:variable>
		
		<xsl:choose>
			<!-- Need to check the namespace instead of the type passed in   -->
			<!-- because we can't be sure they're using the xsd: prefix      -->
			<!-- (sometimes xs: or even s: is used).                         -->
			<xsl:when test="$type-namespace = 'http://www.w3.org/2001/XMLSchema'">
				<!-- It's a builtin type -->
				<xsl:choose>
					<xsl:when test="$type-lookup-nodeset/localName = 'string'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required' or $cardinality = 'optional'">
								<!-- Single string, null if optional and unset -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>        free(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>);</xsl:text>
								<xsl:call-template name="newline"/>

								<xsl:text>        self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = NULL;</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>    }</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of strings -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>        int i;</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>        for(i=0; i&lt;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count; i++) {</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>            if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[i]) {</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>                free(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[i]);</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>            }</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>        }</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>        free(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>);</xsl:text>
								<xsl:call-template name="newline"/>

								<xsl:text>        self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = NULL;</xsl:text>
								<xsl:call-template name="newline"/>

								<xsl:text>        self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count = 0;</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:call-template name="newline"/>
								<xsl:text>    }</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'int' or $type-lookup-nodeset/localName = 'dateTime' or $type-lookup-nodeset/localName = 'boolean'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required'">
								<!-- Single value, set to zero -->
								<xsl:text>    self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = 0;</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:when test="$cardinality = 'optional'">
								<!-- Optional value, set to zero and unset -->
								<xsl:text>    self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = 0;</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>    self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_set = 0;</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of simple values -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>        free(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>);</xsl:text>
								<xsl:call-template name="newline"/>
	
								<xsl:text>        self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = NULL;</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>        self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count = 0;</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>    }</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:message terminate="yes">
							<xsl:text>Unsupported builtin type </xsl:text>
							<xsl:value-of select="string($type)"/>
						</xsl:message>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<!-- It's a complex type -->
				<xsl:variable name="c-struct-name">
					<xsl:call-template name="ct_to_struct_name">
						<xsl:with-param name="type-name" select="$type/@type"></xsl:with-param>
					</xsl:call-template>
				</xsl:variable>
				<xsl:choose>
					<xsl:when test="$cardinality = 'required'">
						<!-- Required, embedded struct -->
						<xsl:text>    </xsl:text>
						<xsl:value-of select="$c-struct-name"/>
						<xsl:text>_destroy(&amp;self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>);</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:when test="$cardinality = 'optional'">
						<!-- Optional, struct pointer -->
						<xsl:text>    if(self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>) {</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:text>        </xsl:text>						
						<xsl:value-of select="$c-struct-name"/>
						<xsl:text>_destroy(self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>);</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:text>        self-&gt;</xsl:text>						
						<xsl:value-of select="$c-name"/>
						<xsl:text> = NULL;</xsl:text>
						<xsl:call-template name="newline"/>
						<xsl:text>    }</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- Complex type array -->
						<xsl:text>    if(self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>) {</xsl:text>
						<xsl:call-template name="newline"/>
						<xsl:text>        int i;</xsl:text>
						<xsl:call-template name="newline"/>
						<xsl:text>        for(i=0; i&lt;self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>_count; i++) {</xsl:text>
						<xsl:call-template name="newline"/>
						<xsl:text>             </xsl:text>						
						<xsl:value-of select="$c-struct-name"/>
						<xsl:text>_destroy(&amp;self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>[i]);</xsl:text>
						<xsl:call-template name="newline"/>

						<xsl:text>        }</xsl:text>
						<xsl:call-template name="newline"/>

						<xsl:text>        free(self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>);</xsl:text>
						<xsl:call-template name="newline"/>

						<xsl:text>        self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text> = NULL;</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:text>        self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>_count = 0;</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:text>    }</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- Handles binding (xml->c) the struct members for each element -->
	<xsl:template name="xsd_type_bind">
		<xsl:param name="type"/> <!-- Either an xsd:element or an xsd:attribute -->
		<xsl:param name="cardinality"/>
		<xsl:param name="is-attribute" select="false"/>
		<xsl:variable name="type-lookup">
			<xsl:call-template name="xsd_type_reference_to_qname">
				<xsl:with-param name="type-name" select="$type/@type"/>
			</xsl:call-template>
		</xsl:variable>
		<!-- Convert it into a nodeset so we can select against it -->
		<xsl:variable name="type-lookup-nodeset" select="exsl:node-set($type-lookup)"/>
		<xsl:variable name="type-namespace" select="string($type-lookup-nodeset/namespace)"/>
		
		<!-- Get the C name for the element -->
		<xsl:variable name="c-name">
			<xsl:call-template name="element_to_c_name">
				<xsl:with-param name="element-name" select="$type/@name"/>
			</xsl:call-template>
		</xsl:variable>
		
		<!-- Get the XML name for the element -->
		<xsl:variable name="xml-name">
			<xsl:value-of select="$type/@name"/>
		</xsl:variable>
		
		<xsl:choose>
			<xsl:when test="$is-attribute = 'false'">
				<xsl:text>        } else if(!strcmp(&quot;</xsl:text>
				<xsl:value-of select="$xml-name"/>
				<xsl:text>", (char*)child-&gt;name)) {</xsl:text>
				<xsl:call-template name="newline"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:text>        {</xsl:text>
				<xsl:call-template name="newline"/>
			</xsl:otherwise>
		</xsl:choose>
		
		<xsl:choose>
			<!-- Need to check the namespace instead of the type passed in   -->
			<!-- because we can't be sure they're using the xsd: prefix      -->
			<!-- (sometimes xs: or even s: is used).                         -->
			<xsl:when test="$type-namespace = 'http://www.w3.org/2001/XMLSchema'">
				<!-- It's a builtin type -->
				
				<!-- Get the value string -->
				<xsl:text>            xmlChar *value;</xsl:text>
				<xsl:call-template name="newline"/>
				<xsl:choose>
					<xsl:when test="$is-attribute = 'true'">
						<xsl:text>            value = xmlGetProp(node, BAD_CAST "</xsl:text>
						<xsl:value-of select="$xml-name"/>
						<xsl:text>");</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:text>            value = xmlNodeGetContent(child);</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:otherwise>
				</xsl:choose>
            	<xsl:text>            if(value) {</xsl:text>
            	<xsl:call-template name="newline"/>
            	
            	<xsl:text>                // c-name: </xsl:text>
				<xsl:value-of select="$c-name"/>
            	<xsl:text> xml-type: </xsl:text>
            	<xsl:value-of select="$type-lookup-nodeset/localName"/>
            	<xsl:text> cardinality: </xsl:text>
            	<xsl:value-of select="$cardinality"/>
				<xsl:call-template name="newline"/>
            	
            	<!-- Parse value based on builtin type (string, int, dateTime, etc) -->
				<xsl:choose>
					<xsl:when test="$type-lookup-nodeset/localName = 'string'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required' or $cardinality = 'optional'">
								<!-- Single string, null if optional and unset -->
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = strdup((char*)value);</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of strings -->							
								<xsl:text>                char **tmp = realloc(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>, (self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count + 1) * sizeof(char*));</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>                if(!tmp) {</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    fprintf(stderr, &quot;realloc failed!\n&quot;);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    xmlFree(value);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    return;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                }</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = tmp;</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count++] = strdup((char*)value);</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'int'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required'">
								<!-- Single int -->
								<xsl:text>                sscanf((char*)value, "%" SCNd64, &amp;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>);</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:when test="$cardinality = 'optional'">
								<!-- Optional int, parse and mark as _set -->
								<xsl:text>                if(sscanf((char*)value, "%" SCNd64, &amp;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>)) {</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_set = 1;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                }</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of int -->
								<xsl:text>                int64_t tmp;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                if(sscanf((char*)value, "%" SCNd64, &amp;tmp)) {</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    // Parse success, attempt realloc</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    int *tmparr = realloc(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>, (self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count+1) * sizeof(int64_t));</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    if(!tmparr) {</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                        fprintf(stderr, &quot;Realloc failed&quot;);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                        xmlFree(value);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                        return;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    }</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = tmparr;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count++] = tmp;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                }</xsl:text><xsl:call-template name="newline"/>

							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>	
					<xsl:when test="$type-lookup-nodeset/localName = 'dateTime'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required'">
								<!-- Single dateTime -->
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = datetime_parse(value);</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:when test="$cardinality = 'optional'">
								<!-- Optional dateTime, parse and mark as _set -->
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = datetime_parse(value);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_set = 1;</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of dateTime -->
								<xsl:text>                int *tmparr = realloc(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>, (self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count+1) * sizeof(time_t));</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                if(!tmparr) {</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    fprintf(stderr, &quot;Realloc failed&quot;);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    xmlFree(value);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    return;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                }</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = tmparr;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count++] = datetime_parse(value);</xsl:text><xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>

					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'boolean'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required'">
								<!-- Single bool -->
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = strcmp("true", (char*)value) == 0;</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:when test="$cardinality = 'optional'">
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = strcmp("true", (char*)value) == 0;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_set = 1;</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of bool -->
								<xsl:text>                int *tmparr = realloc(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>, (self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count+1) * sizeof(int));</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                if(!tmparr) {</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    fprintf(stderr, &quot;Realloc failed&quot;);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    xmlFree(value);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                    return;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                }</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = tmparr;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count++] = strcmp("true", (char*)value) == 0;</xsl:text><xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:message terminate="yes">
							<xsl:text>Unsupported builtin type </xsl:text>
							<xsl:value-of select="string($type)"/>
						</xsl:message>
					</xsl:otherwise>
				</xsl:choose>
				
				<xsl:text>                xmlFree(value);</xsl:text>
				<xsl:call-template name="newline"/>
				<xsl:text>            }</xsl:text>
				<xsl:call-template name="newline"/>

			</xsl:when>
			<xsl:otherwise>
				<!-- It's a complex type -->
				<xsl:variable name="ct-struct-name">
					<xsl:call-template name="ct_to_struct_name">
						<xsl:with-param name="type-name" select="$type/@type"></xsl:with-param>
					</xsl:call-template>
				</xsl:variable>
				
				<xsl:choose>
					<xsl:when test="$cardinality = 'required'">
						<!-- Required, embedded struct -->
					  	<xsl:text>            </xsl:text>
						<xsl:value-of select="$ct-struct-name"/>
					  	<xsl:text>_bind(&amp;self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>, child);</xsl:text><xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:when test="$cardinality = 'optional'">
						<!-- Optional, struct pointer -->
					  	<xsl:text>            if(!self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>) {</xsl:text><xsl:call-template name="newline"/>
					  	<xsl:text>                self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text> = malloc(sizeof(</xsl:text>
						<xsl:value-of select="$ct-struct-name"/>
					  	<xsl:text>));</xsl:text><xsl:call-template name="newline"/>
					  	<xsl:text>                if(!self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>) {</xsl:text><xsl:call-template name="newline"/>
						<xsl:text>                    fprintf(stderr, &quot;Malloc failed&quot;);</xsl:text><xsl:call-template name="newline"/>
						<xsl:text>                    return;</xsl:text><xsl:call-template name="newline"/>
						<xsl:text>                }</xsl:text><xsl:call-template name="newline"/>
						<xsl:text>            }</xsl:text><xsl:call-template name="newline"/>
						
						<xsl:text>            </xsl:text>
						<xsl:value-of select="$ct-struct-name"/>
					  	<xsl:text>_init(self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>);</xsl:text><xsl:call-template name="newline"/>

						<xsl:text>            </xsl:text>
						<xsl:value-of select="$ct-struct-name"/>
					  	<xsl:text>_bind(self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>, child);</xsl:text><xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- Complex type array -->
								<xsl:text>            </xsl:text>
								<xsl:value-of select="$ct-struct-name"/>
							  	<xsl:text> *tmparr = realloc(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>, (self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count+1) * sizeof(</xsl:text>
								<xsl:value-of select="$ct-struct-name"/>
							  	<xsl:text>));</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>            if(!tmparr) {</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                fprintf(stderr, &quot;Realloc failed&quot;);</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>                return;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>            }</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>            self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text> = tmparr;</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>            </xsl:text>
								<xsl:value-of select="$ct-struct-name"/>
							  	<xsl:text>_init(&amp;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count]);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>            </xsl:text>
								<xsl:value-of select="$ct-struct-name"/>
							  	<xsl:text>_bind(&amp;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count], child);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>            self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count++;</xsl:text><xsl:call-template name="newline"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
		
		<xsl:if test="$is-attribute = 'true'">
			<xsl:text>        }</xsl:text>
			<xsl:call-template name="newline"/>
		</xsl:if>
		
	</xsl:template>
	
	<!-- Unbinds a complex type into an element -->
	<xsl:template name="xsd_ct_unbind">
		<xsl:param name="c-value"/>
		<xsl:param name="c-type"/>
		<xsl:param name="xml-name"/>
		<xsl:param name="xml-namespace"/>
		<xsl:param name="xml-namespace-prefix"/>
		<xsl:param name="indent"/>
		
		<xsl:choose>
			<xsl:when test="$xml-namespace = ''">
				<xsl:value-of select="$indent"/>
				<xsl:text>xmlNodePtr child = xmlNewNode(NULL, BAD_CAST &quot;</xsl:text>
				<xsl:value-of select="$xml-name"/>
				<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:value-of select="$indent"/>
				<xsl:value-of select="$c-type"/>
				<xsl:text>_unbind(</xsl:text>
				<xsl:value-of select="$c-value"/>
				<xsl:text>, doc, child);</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:value-of select="$indent"/>
				<xsl:text>xmlAddChild(node, child);</xsl:text><xsl:call-template name="newline"/>
			</xsl:when>
			<xsl:otherwise>
				<!-- Set the namespace too -->
				<xsl:value-of select="$indent"/>
				<xsl:text>xmlNsPtr ns = lookupNamespace(doc, &quot;</xsl:text>
				<xsl:value-of select="$xml-namespace"/>
				<xsl:text>&quot;, &quot;</xsl:text>
				<xsl:value-of select="$xml-namespace-prefix"/>
				<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:value-of select="$indent"/>
				<xsl:text>xmlNodePtr child = xmlNewNode(ns, BAD_CAST &quot;</xsl:text>
				<xsl:value-of select="$xml-name"/>
				<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:value-of select="$indent"/>
				<xsl:value-of select="$c-type"/>
				<xsl:text>_unbind(</xsl:text>
				<xsl:value-of select="$c-value"/>
				<xsl:text>, doc, child);</xsl:text><xsl:call-template name="newline"/>
				
				<xsl:value-of select="$indent"/>
				<xsl:text>xmlAddChild(node, child);</xsl:text><xsl:call-template name="newline"/>
			</xsl:otherwise>
		</xsl:choose>
		
	</xsl:template>
	
	<!-- Unbinds a value into either an element or attribute -->
	<xsl:template name="xsd_value_unbind">
		<xsl:param name="c-value"/>
		<xsl:param name="xml-name"/>
		<xsl:param name="is-attribute"/>
		<xsl:param name="xml-namespace"/>
		<xsl:param name="xml-namespace-prefix"/>
		<xsl:param name="indent"/>
		
		<xsl:choose>
			<xsl:when test="$is-attribute = 'true'">
				<xsl:value-of select="$indent"/>
				<xsl:text>xmlNewProp(node, BAD_CAST &quot;</xsl:text>
				<xsl:value-of select="$xml-name"/>
				<xsl:text>&quot;, BAD_CAST </xsl:text>
				<xsl:value-of select="$c-value"/>
				<xsl:text>);</xsl:text>
				<xsl:call-template name="newline"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:choose>
					<xsl:when test="$xml-namespace = ''">
						<xsl:value-of select="$indent"/>
						<xsl:text>xmlNodePtr child = xmlNewNode(NULL, BAD_CAST &quot;</xsl:text>
						<xsl:value-of select="$xml-name"/>
						<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
						
						<xsl:value-of select="$indent"/>
						<xsl:text>xmlNodeSetContent(child, BAD_CAST </xsl:text>
						<xsl:value-of select="$c-value"/>
						<xsl:text>);</xsl:text><xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- Set the namespace too -->
						<xsl:value-of select="$indent"/>
						<xsl:text>xmlNsPtr ns = lookupNamespace(doc, &quot;</xsl:text>
						<xsl:value-of select="$xml-namespace"/>
						<xsl:text>&quot;, &quot;</xsl:text>
						<xsl:value-of select="$xml-namespace-prefix"/>
						<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
						
						<xsl:value-of select="$indent"/>
						<xsl:text>xmlNodePtr child = xmlNewNode(ns, BAD_CAST &quot;</xsl:text>
						<xsl:value-of select="$xml-name"/>
						<xsl:text>&quot;);</xsl:text><xsl:call-template name="newline"/>
						
						<xsl:value-of select="$indent"/>
						<xsl:text>xmlNodeSetContent(child, BAD_CAST </xsl:text>
						<xsl:value-of select="$c-value"/>
						<xsl:text>);</xsl:text><xsl:call-template name="newline"/>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:value-of select="$indent"/>
				<xsl:text>xmlAddChild(node, child);</xsl:text><xsl:call-template name="newline"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<!-- Handles unbinding (c->xml) the struct members for each element -->
	<xsl:template name="xsd_type_unbind">
		<xsl:param name="type"/> <!-- Either an xsd:element or an xsd:attribute -->
		<xsl:param name="cardinality"/>
		<xsl:param name="is-attribute"/>
		<xsl:variable name="type-lookup">
			<xsl:call-template name="xsd_type_reference_to_qname">
				<xsl:with-param name="type-name" select="$type/@type"/>
			</xsl:call-template>
		</xsl:variable>
		<!-- Convert it into a nodeset so we can select against it -->
		<xsl:variable name="type-lookup-nodeset" select="exsl:node-set($type-lookup)"/>
		<xsl:variable name="type-namespace" select="string($type-lookup-nodeset/namespace)"/>
		
		<!-- Get the C name for the element -->
		<xsl:variable name="c-name">
			<xsl:call-template name="element_to_c_name">
				<xsl:with-param name="element-name" select="$type/@name"/>
			</xsl:call-template>
		</xsl:variable>
		
		<!-- Get the full XML name for the element -->
		<xsl:variable name="xml-name-fragment">
			<xsl:call-template name="xsd_name_reference_to_qname">
				<xsl:with-param name="type-name" select="$type/@name"/>
			</xsl:call-template>
		</xsl:variable>
		<!-- Convert it into a nodeset so we can select against it -->
		<xsl:variable name="xml-name" select="exsl:node-set($xml-name-fragment)"/>
		
		
		<xsl:choose>
			<!-- Need to check the namespace instead of the type passed in   -->
			<!-- because we can't be sure they're using the xsd: prefix      -->
			<!-- (sometimes xs: or even s: is used).                         -->
			<xsl:when test="$type-namespace = 'http://www.w3.org/2001/XMLSchema'">
				<!-- It's a builtin type -->
				<xsl:choose>
					<xsl:when test="$type-lookup-nodeset/localName = 'string'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required' or $cardinality = 'optional'">
								<!-- Single string, null if optional and unset -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="concat('self->', $c-name)"/>
									<xsl:with-param name="indent" select="'        '"/>
								</xsl:call-template>
								
								<xsl:text>    }</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of strings -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>        int i;</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>        for(i=0; i&lt;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count; i++) {</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>            if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[i]) {</xsl:text>
								<xsl:call-template name="newline"/>

								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="concat('self->', $c-name, '[i]')"/>
									<xsl:with-param name="indent" select="'                '"/>
								</xsl:call-template>

								<xsl:text>            }</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>        }</xsl:text>
								<xsl:call-template name="newline"/>
								<xsl:text>    }</xsl:text>
								<xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'int'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required'">
								<!-- Single int value -->
								<xsl:text>    char buffer[255];</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>    snprintf(buffer, 255, &quot;%&quot; PRId64, self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'buffer'"/>
									<xsl:with-param name="indent" select="'    '"/>
								</xsl:call-template>
								
							</xsl:when>
							<xsl:when test="$cardinality = 'optional'">
								<!-- Optional int, check _set member -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_set) {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>        char buffer[255];</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>        snprintf(buffer, 255, &quot;%&quot; PRId64, self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'buffer'"/>
									<xsl:with-param name="indent" select="'        '"/>
								</xsl:call-template>
								
								<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of int values -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>        int i;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>        for(i=0; i&lt;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count; i++) {</xsl:text>
								<xsl:call-template name="newline"/>
	
								<xsl:text>            char buffer[255];</xsl:text><xsl:call-template name="newline"/>

								<xsl:text>            snprintf(buffer, 255, &quot;%&quot; PRId64, self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[i]);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'buffer'"/>
									<xsl:with-param name="indent" select="'            '"/>
								</xsl:call-template>
								
								<xsl:text>            }</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>        }</xsl:text><xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'dateTime'">
					<xsl:choose>
							<xsl:when test="$cardinality = 'required'">
								<!-- Single dateTime value -->
								<xsl:text>    char buffer[255];</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>    struct tm a;</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>    gmtime_r(&amp;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>, &amp;a);</xsl:text><xsl:call-template name="newline"/>

								<xsl:text>    strftime(buffer, 255, "%FT%TZ", &amp;a);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'buffer'"/>
									<xsl:with-param name="indent" select="'    '"/>
								</xsl:call-template>
								
							</xsl:when>
							<xsl:when test="$cardinality = 'optional'">
								<!-- Optional dateTime, check _set member -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_set) {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>        char buffer[255];</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>        struct tm a;</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>        gmtime_r(&amp;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>, &amp;a);</xsl:text><xsl:call-template name="newline"/>

								<xsl:text>        strftime(buffer, 255, "%FT%TZ", &amp;a);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'buffer'"/>
									<xsl:with-param name="indent" select="'        '"/>
								</xsl:call-template>
								
								<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of dateTime values -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>        int i;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>        for(i=0; i&lt;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count; i++) {</xsl:text>
								<xsl:call-template name="newline"/>
	
								<xsl:text>            char buffer[255];</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>            struct tm a;</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>            gmtime_r(&amp;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[i], &amp;a);</xsl:text><xsl:call-template name="newline"/>

								<xsl:text>            strftime(buffer, 255, "%FT%TZ", &amp;a);</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'buffer'"/>
									<xsl:with-param name="indent" select="'            '"/>
								</xsl:call-template>
								
								<xsl:text>        }</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>
					
					</xsl:when>
					<xsl:when test="$type-lookup-nodeset/localName = 'boolean'">
						<xsl:choose>
							<xsl:when test="$cardinality = 'required'">
								<!-- Single boolean value -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'&quot;true&quot;'"/>
									<xsl:with-param name="indent" select="'        '"/>
								</xsl:call-template>
								
								<xsl:text>        } else {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'&quot;false&quot;'"/>
									<xsl:with-param name="indent" select="'        '"/>
								</xsl:call-template>
								
								<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:when test="$cardinality = 'optional'">
								<!-- Optional boolean, check _set member -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_set) {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>        if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'&quot;true&quot;'"/>
									<xsl:with-param name="indent" select="'            '"/>
								</xsl:call-template>
								
								<xsl:text>        } else {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'&quot;false&quot;'"/>
									<xsl:with-param name="indent" select="'            '"/>
								</xsl:call-template>
								
								<xsl:text>        }</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
							</xsl:when>
							<xsl:otherwise>
								<!-- Array of boolean values -->
								<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>) {</xsl:text>
								<xsl:call-template name="newline"/>
								
								<xsl:text>        int i;</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>        for(i=0; i&lt;self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>_count; i++) {</xsl:text>
								<xsl:call-template name="newline"/>
	
								<xsl:text>        if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
								<xsl:text>[i]) {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'&quot;true&quot;'"/>
									<xsl:with-param name="indent" select="'                '"/>
								</xsl:call-template>
								
								<xsl:text>            } else {</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:call-template name="xsd_value_unbind">
									<xsl:with-param name="is-attribute" select="$is-attribute"/>
									<xsl:with-param name="xml-name" select="$xml-name/localName"/>
									<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
									<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
									<xsl:with-param name="c-value" select="'&quot;false&quot;'"/>
									<xsl:with-param name="indent" select="'                '"/>
								</xsl:call-template>
								
								<xsl:text>            }</xsl:text><xsl:call-template name="newline"/>
								
								<xsl:text>        }</xsl:text><xsl:call-template name="newline"/>
								<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
							</xsl:otherwise>
						</xsl:choose>

					</xsl:when>
					<xsl:otherwise>
						<xsl:message terminate="yes">
							<xsl:text>Unsupported builtin type </xsl:text>
							<xsl:value-of select="string($type)"/>
						</xsl:message>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<!-- It's a complex type -->
				<xsl:variable name="c-type">
					<xsl:call-template name="ct_to_struct_name">
						<xsl:with-param name="type-name" select="$type/@type"></xsl:with-param>
					</xsl:call-template>
				</xsl:variable>
				<xsl:choose>
					<xsl:when test="$cardinality = 'required'">
						<!-- Required, embedded struct -->
						<!-- Note that we put an extra brace here to scope   -->
						<!-- the local variables created by xsd_ct_unbind.   -->
						<xsl:text>    {</xsl:text><xsl:call-template name="newline"/>
						<xsl:call-template name="xsd_ct_unbind">
							<xsl:with-param name="c-type" select="$c-type"/>
							<xsl:with-param name="c-value" select="concat('&amp;self-&gt;', $c-name)"/>
							<xsl:with-param name="xml-name" select="$xml-name/localName"/>
							<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
							<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
							<xsl:with-param name="indent" select="'        '"/>
						</xsl:call-template>
						<xsl:text>    }</xsl:text><xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:when test="$cardinality = 'optional'">
						<!-- Optional, struct pointer -->
						<xsl:text>    if(self-&gt;</xsl:text>
								<xsl:value-of select="$c-name"/>
						<xsl:text>) {</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:call-template name="xsd_ct_unbind">
							<xsl:with-param name="c-type" select="$c-type"/>
							<xsl:with-param name="c-value" select="concat('self-&gt;', $c-name)"/>
							<xsl:with-param name="xml-name" select="$xml-name/localName"/>
							<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
							<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
							<xsl:with-param name="indent" select="'        '"/>
						</xsl:call-template>

						<xsl:text>    }</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:when>
					<xsl:otherwise>
						<!-- Complex type array -->
						<xsl:text>    if(self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>) {</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:text>        int i;</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:text>        for(i=0; i&lt;self-&gt;</xsl:text>
						<xsl:value-of select="$c-name"/>
						<xsl:text>_count; i++) {</xsl:text>
						<xsl:call-template name="newline"/>
						
						<xsl:call-template name="xsd_ct_unbind">
							<xsl:with-param name="c-type" select="$c-type"/>
							<xsl:with-param name="c-value" select="concat('&amp;self-&gt;', $c-name, '[i]')"/>
							<xsl:with-param name="xml-name" select="$xml-name/localName"/>
							<xsl:with-param name="xml-namespace-prefix" select="$xml-name/prefix"/>
							<xsl:with-param name="xml-namespace" select="$xml-name/namespace"/>
							<xsl:with-param name="indent" select="'            '"/>
						</xsl:call-template>
						
						<xsl:text>        }</xsl:text>
						<xsl:call-template name="newline"/>
						<xsl:text>    }</xsl:text>
						<xsl:call-template name="newline"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	
	<!-- Defines some helper functions in C -->
	<xsl:template name="c-helper-functions">
<xsl:text>
static time_t
datetime_parse(xmlChar *value) {
    time_t tt;
    struct tm t;
    char buffer[255];
    char *decimal;


    memset(&amp;t, 0, sizeof(struct tm));
    size_t strsz;
    strsz = strlen((char*)value);
    if(value[strsz-1] == 'Z') {
        strcpy(buffer, (char*)value);
        buffer[strlen(buffer)-1] = 0;
        // If there's milliseconds in the timestamp, strip it out.
        decimal = strchr(buffer, '.');
        if(decimal) {
            *decimal = 0;
        }
        strcat(buffer, "GMT");
        if(!strptime(buffer, "%FT%T%Z", &amp;t)) {
            fprintf(stderr, "Could not parse dateTime value %s\n", buffer);
            return 0;
        }
        tt = mktime(&amp;t);
    } else if(value[strsz-5] == '+' || value[strsz-5] == '-') {
        if(!strptime((char*)value, "%FT%T%z", &amp;t)) {
            fprintf(stderr, "Could not parse dateTime value %s\n", buffer);
            return 0;
        }
        tt = mktime(&amp;t);
    } else {
        if(!strptime((char*)value, "%FT%T%Z", &amp;t)) {
            fprintf(stderr, "Could not parse dateTime value %s\n", buffer);
            return 0;
        }
        tt = mktime(&amp;t);
    } return tt;
}

xmlNsPtr
lookup_namespace(xmlDocPtr doc, xmlNodePtr node, char *namespace_href, char *namespace_prefix) {
    xmlNsPtr ns = xmlSearchNsByHref(doc, node, BAD_CAST namespace_href);
    if(!ns) {
        // Namespace not registered yet.  Register it on the root node.
        xmlNodePtr root = xmlDocGetRootElement(doc);
        ns = xmlNewNs(root, BAD_CAST namespace_href, BAD_CAST namespace_prefix);
    }
    return ns;
}
</xsl:text>
	</xsl:template>

</xsl:stylesheet>