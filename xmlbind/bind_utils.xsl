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
 
<!-- 
  This stylesheet includes generic utility templates that can be used for
  generating XML binding code in any language. 
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"	
xmlns:exsl="http://exslt.org/common" 
	xmlns:xsd="http://www.w3.org/2001/XMLSchema"
	xmlns:str="http://exslt.org/strings" 
	xmlns:set="http://exslt.org/sets"
	extension-element-prefixes="str exsl set">
	
	<!-- 
	     Template to generate a newline.  Don't modify the indentation here
	     or you'll get extra whitespace in your output.
	-->
	<xsl:template name="newline">
<xsl:text>
</xsl:text>
	</xsl:template>
		
	<!-- Converts a list of tokens from str:split into PascalCase -->
	<xsl:template name="token_list_to_pascal_case">
		<xsl:param name="tokens"/>
		<xsl:for-each select="$tokens">
			<xsl:variable name="upper">
				<xsl:value-of select="translate(substring(text(),1,1), 
					'abcdefghijklmnopqrstuvwxyz', 
					'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
			</xsl:variable>
			<xsl:value-of select="concat($upper, substring(text(),2))"/>
		</xsl:for-each>
	</xsl:template>
	
	<!-- Converts a list of tokens from str:split into camelCase -->
	<xsl:template name="token_list_to_camel_case">
		<xsl:param name="tokens"/>
		<xsl:variable name="first_token" select="$tokens[1]"/>
		<xsl:variable name="other_tokens" select="$tokens[position() != 1]"/>
		<!-- Make sure first letter of first token is lowercase. -->
		<xsl:variable name="lower" select="translate(substring($first_token/text(),1,1), 
				'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
				'abcdefghijklmnopqrstuvwxyz')">
		</xsl:variable>
		<xsl:value-of select="concat($lower, substring($first_token/text(),2))"/>
		<xsl:for-each select="$other_tokens">
			<xsl:variable name="upper" select="translate(substring(text(),1,1), 
				'abcdefghijklmnopqrstuvwxyz',
				'ABCDEFGHIJKLMNOPQRSTUVWXYZ')">
			</xsl:variable>
			<xsl:value-of select="concat($upper, substring(text(),2))"/>
		</xsl:for-each>
	</xsl:template>
	
	<!-- Converts an xsd *type* reference (e.g. 'xsd:dateTime') to an element-->
	<!-- containing a <namespace>, <prefix>, and <localName> element.  If    -->
	<!-- the namespace equals the documents 'targetNamespace', a third       -->
	<!-- element with <targetNamepace>true</targetNamespace> will be         -->
	<!-- appended.                                                           -->
	<xsl:template name="xsd_type_reference_to_qname">
		<xsl:param name="type-name"/>
		<xsl:variable name="prefix" select="substring-before($type-name, ':')"/>
		<xsl:variable name="namespace">
			<xsl:choose>
				<xsl:when test="contains($type-name, ':')">
					<xsl:call-template name="xml_prefix_to_namespace">
						<xsl:with-param name="prefix" select="$prefix"/>
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>
					<!-- No namespace specified, get default ns -->
					<xsl:call-template name="xml_prefix_to_namespace">
						<xsl:with-param name="prefix" select="''"/>
					</xsl:call-template>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="localName">
			<xsl:choose>
				<xsl:when test="contains($type-name, ':')">
					<xsl:value-of select="substring-after($type-name, ':')"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="$type-name"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:element name="namespace" >
			<xsl:value-of select="$namespace"/>
		</xsl:element>
		<xsl:element name="localName">
			<xsl:value-of select="string($localName)"/>
		</xsl:element>
		<xsl:element name="prefix">
			<xsl:value-of select="$prefix"/>
		</xsl:element>
		<xsl:if test="$namespace = string(/xsd:schema/@targetNamespace)">
			<xsl:element name="targetNamespace">
				<xsl:text>true</xsl:text>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	
	<!-- Converts an xsd *name* reference(e.g.'tns:fooBarType') to an element-->
	<!-- containing a <namespace> and <localName> element.  If the namespace -->
	<!-- prefix is blank, the targetNamespace will be used (if any).         -->
	<xsl:template name="xsd_name_reference_to_qname">
		<xsl:param name="type-name"/>
		<xsl:variable name="prefix" select="substring-before($type-name, ':')"/>
		<xsl:variable name="namespace">
			<xsl:choose>
				<xsl:when test="contains($type-name, ':')">
					<xsl:call-template name="xml_prefix_to_namespace">
						<xsl:with-param name="prefix" select="$prefix"/>
					</xsl:call-template>
				</xsl:when>
				<xsl:otherwise>
					<!-- No namespace specified, get targetNamespace -->
					<xsl:value-of select="/xsd:schema/@targetNamespace"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="localName">
			<xsl:choose>
				<xsl:when test="contains($type-name, ':')">
					<xsl:value-of select="substring-after($type-name, ':')"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="$type-name"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:element name="namespace" >
			<xsl:value-of select="$namespace"/>
		</xsl:element>
		<xsl:element name="localName">
			<xsl:value-of select="string($localName)"/>
		</xsl:element>
		<xsl:element name="prefix">
			<xsl:value-of select="$prefix"/>
		</xsl:element>
		<xsl:if test="$namespace = string(/xsd:schema/@targetNamespace)">
			<xsl:element name="targetNamespace">
				<xsl:text>true</xsl:text>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	
	
	<!-- Converts an xml prefix (e.g. 'xsd:'), without the ':' to a          -->
	<!-- fully-qualified namespace (e.g. 'http://www.w3.org/2001/XMLSchema') -->
	<xsl:template name="xml_prefix_to_namespace">
		<xsl:param name="prefix"/>
		<xsl:for-each select="set:distinct(/*/namespace::*)">
			<xsl:if test="name(.) = $prefix">
				<xsl:value-of select="string(.)"/>
			</xsl:if>
		</xsl:for-each>
	</xsl:template>
	
	<!-- Translates a string to uppercase -->
	<xsl:template name="uppercase">
		<xsl:param name="value"/>
		<xsl:value-of select="translate($value, 'abcdefghijklmnopqrstuvwxyz', 
			'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"></xsl:value-of>
	</xsl:template>
	
	<!-- Looks up a set of complex types by name.  For each string type name 
	     in $type_names that is not in $processed_types, the XSD document in 
	     $root will be searched for a corresponding complexType.  If the type 
	     is located it will be returned in the resulting document fragment. -->
	<xsl:template name="ct_names_to_types">
		<xsl:param name="type_names"/>
		<xsl:param name="processed_types" select="/.."/>
		<xsl:param name="root"/>
		<xsl:for-each select="$type_names">
			<xsl:variable name="type_name" select="string(.)"/>
			<xsl:variable name="type-lookup">
				<xsl:call-template name="xsd_type_reference_to_qname">
					<xsl:with-param name="type-name" select="$type_name"/>
				</xsl:call-template>
			</xsl:variable>
			<!-- Convert it into a nodeset so we can select against it -->
			<xsl:variable name="type-lookup-nodeset" select="exsl:node-set($type-lookup)"/>		
			
			<xsl:if test="count($processed_types[@name = $type_name]) &lt; 1">
				<xsl:variable name="xx" select="concat('looking for ', $type_name)"/>
				<xsl:choose>
					<!-- If the type name has no namespace or is in an       -->
					<!-- explicit namespace, it will probably match exactly  -->
					<xsl:when test="$root/xsd:schema/xsd:complexType[@name = $type_name]">
						<xsl:copy-of select="$root/xsd:schema/xsd:complexType[@name = $type_name]"/>
					</xsl:when>
					<!-- If the element is in targetNamespace, it may be     -->
					<!-- present with @name that doesn't contain the same    -->
					<!-- prefix as the reference in $type_name.              -->
					<xsl:when test="$type-lookup-nodeset/targetNamespace = 'true' and $root/xsd:schema/xsd:complexType[@name = $type-lookup-nodeset/localName]">
						<xsl:copy-of select="$root/xsd:schema/xsd:complexType[@name = $type-lookup-nodeset/localName]"/>
					</xsl:when>
				</xsl:choose>
			</xsl:if>
		</xsl:for-each>
	</xsl:template>
	
	

</xsl:stylesheet>