<?xml version="1.0"?>
<!-- 
  Stylesheet to convert the property reference in raw XML code to a
  nice DocBook version. (c) 2003 Chris Schlaeger <cs@suse.de>
-->
<xsl:stylesheet version="1.0"
 xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

 <xsl:template match="properties">
  <chapter><title>Property Reference</title>
  <xsl:apply-templates select="property"/>
  </chapter>
 </xsl:template>

 <xsl:template match="property">
  <sect1 id="PROPERTY_{@id}">
   <title><xsl:value-of select="@name"/> 
   <xsl:apply-templates select="attributes" mode="head"/></title>
  <para>
  <informaltable>
  <tgroup cols='4' align='left'>
  <colspec colname='c1'/>
  <colspec colname='c2'/>
  <colspec colname='c3'/>
  <colspec colname='c4'/>
  <spanspec spanname='sp1' namest='c2' nameend='c4'/>
  <thead>
   <row>
    <entry namest="c1" nameend="c4" align="left">
    <xsl:value-of select="@name"/> 
    <xsl:apply-templates select="attributes" mode="head"/></entry>
   </row>
  </thead>
  <tbody>
  <xsl:apply-templates select="descr"/>
  <xsl:apply-templates select="attributes" mode="body"/>
  <xsl:apply-templates select="optattributes"/>
  <row>
   <entry>Context</entry>
   <entry spanname="sp1">
    <xsl:variable name="prop" select="@id"/>
    <!-- Search all properties and list those who have the current
         property listed as optional attribute. -->
    <xsl:for-each select="/properties/*">
     <xsl:variable name="cntx" select="@id"/>
     <xsl:for-each select="optattributes/*">
      <xsl:if test=".=$prop">
       <xsl:choose>
        <xsl:when test="$prop=$cntx">
         <varname><xsl:value-of select="$cntx"/></varname>
        </xsl:when>
        <xsl:otherwise>
         <link linkend="PROPERTY_{$cntx}"><xsl:value-of
          select="/properties/property[@id = $cntx]/@name"/></link>
        </xsl:otherwise>
       </xsl:choose>
       <xsl:if test="1">, </xsl:if> 
      </xsl:if>
     </xsl:for-each>
    </xsl:for-each>
   </entry>
  </row>
  <row>
   <entry>Inheritable</entry>
   <entry>
    <xsl:value-of select="../@inheritable"/>
   </entry>
   <entry>Scenario Spec.</entry>
   <entry>
    <xsl:value-of select="../@scenario"/>
   </entry>
  </row>
  <xsl:apply-templates select="seealso"/>
  </tbody>
  </tgroup>
  </informaltable>
  </para>
  <xsl:apply-templates select="freestyle"/>
  <xsl:apply-templates select="example"/>
  </sect1>
 </xsl:template>

 <xsl:template match="descr">
  <row>
   <entry>Description</entry>
   <entry spanname="sp1"><xsl:copy-of select="para"/></entry>
  </row>
 </xsl:template>

 <xsl:template match="attributes" mode="head">
  <xsl:apply-templates select="attr" mode="head"/>
 </xsl:template>

 <xsl:template match="attr" mode="head">
   <xsl:if test="@optional = 1">[ </xsl:if>
   &lt;<xsl:value-of select="@name"/>&gt;
  <xsl:if test="@list = 1">
   [, &lt;<xsl:value-of select="@name"/>&gt; ... ]
  </xsl:if>
  <xsl:if test="@optional = 1"> ]</xsl:if>
 </xsl:template>

 <xsl:template match="attributes" mode="body">
  <row>
   <entry morerows="{count(*)}">Attributes</entry>
   <entry align="center">Name</entry>
   <entry align="center">Type</entry>
   <entry align="center">Description</entry>
  </row>
  <xsl:apply-templates select="attr" mode="body"/>
 </xsl:template>

 <xsl:template match="attr" mode="body">
  <row> 
   <entry><xsl:value-of select="@name"/></entry>
   <entry><link linkend="TYPE_{@type}"><xsl:value-of
           select="@type"/></link></entry>
   <entry>
    <xsl:copy-of select="para"/>
   </entry>
  </row>
 </xsl:template>

 <xsl:template match="optattributes">
  <row>
   <entry>Optional Attributes</entry>
   <entry spanname="sp1"><xsl:apply-templates select="optattr"/></entry>
  </row>
 </xsl:template>

 <xsl:template match="optattr">
  <xsl:choose>
   <xsl:when test=".=../../@name">
    <varname><xsl:value-of select="."/></varname>
   </xsl:when>
   <xsl:otherwise>
    <link linkend="PROPERTY_{.}"><xsl:value-of
     select="../../../property[@id = current()]/@name"/></link>
   </xsl:otherwise>
  </xsl:choose>
  <xsl:if test="position() != last()">, </xsl:if>
 </xsl:template>

 <xsl:template match="seealso">
  <row>
   <entry>See also</entry>
   <entry spanname="sp1"><xsl:apply-templates select="also"/></entry>
  </row>
 </xsl:template>

 <xsl:template match="also">
  <xsl:choose>
   <xsl:when test=".=../../@name">
    <varname><xsl:value-of select="."/></varname>
   </xsl:when>
   <xsl:otherwise>
    <link linkend="PROPERTY_{.}"><xsl:value-of
     select="../../../property[@id = current()]/@name"/></link>
   </xsl:otherwise>
  </xsl:choose>
  <xsl:if test="position() != last()">, </xsl:if>
 </xsl:template>

 <xsl:template match="freestyle">
  <xsl:copy-of select="*"/>
 </xsl:template>

 <xsl:template match="example">
  <para><screen>
   <xsl:value-of select="."/>
  </screen></para>
 </xsl:template>

</xsl:stylesheet>

