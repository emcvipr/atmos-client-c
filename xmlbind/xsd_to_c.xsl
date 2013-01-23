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
 
 <!-- This stylesheet takes an input XML schema document (XSD) and generates
      a header and source file in C99 that will implement XML marshalling and
      unmarshalling code for XML to C structures and back using libxml2.  This 
      stylesheet is compatible with "xsltproc" processor shipped with 
      libxml2. -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="text" omit-xml-declaration="yes"/>
	
	<xsl:include href="bind_utils.xsl"/>
	<xsl:include href="c_impl.xsl"/>
	
	<!-- Prefix to apply to elements.  Optional but recommended.  If not set
	     the XML complex type names will be defined as-is -->
	<xsl:param name="prefix"/>
	<!-- Module name.  Used for naming the .h and .c files.  Required. -->
	<xsl:param name="module-name"/>
	
	<xsl:template match="/">
		<xsl:if test="not($module-name)">
			<xsl:message terminate="yes">The parameter module-name is required.  This parameter sets the name of the output .c and .h files, e.g. &apos;my-module&apos;</xsl:message>
		</xsl:if>
		<xsl:call-template name="xsd-to-c">
			<xsl:with-param name="p-module-name" select="$module-name"/>
		</xsl:call-template>
	</xsl:template>

</xsl:stylesheet>