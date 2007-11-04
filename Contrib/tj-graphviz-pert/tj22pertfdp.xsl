<?xml version="1.0" encoding="UTF-8"?>
<!--
  XSLT stylesheet for converting TaskJuggler XML v2 report to Pert graph in
  Graphviz' dot format, for fdp output (since node - cluster edges are not
  supported with other renderers)

  should be used for instance as follow (with xmlsoft.org's libxslt):
  zcat v2.tjx | xsltproc -novalid tj22pertfdp.xsl - | fdp -T png > v2fdp.png
  warning: the correct novalid option has two leading dashes (but this can
  not be put in an xml comment such as this one)

  (c) 2007 Grégoire Barbier <gb@gbarbier.org>
  This software is licensed under the terms of the GNU General Public Licence
  version 2, see http://www.gnu.org/licenses/gpl.html
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

 <xsl:output method="text" indent="yes" encoding="UTF-8"/>

 <xsl:key name="task" match="task" use="@id"/>

 <xsl:template match="/taskjuggler">
  <xsl:text>graph g {
    node[shape=box];
  </xsl:text>
   <xsl:apply-templates select="project"/>
   <xsl:apply-templates select="taskList"/>
   <xsl:apply-templates select="descendant::depends"/>
  <xsl:text>}</xsl:text>
 </xsl:template>

 <xsl:template match="project">
  <xsl:text>graph [label="</xsl:text>
  <xsl:value-of select="@name"/>
  <xsl:text>\n</xsl:text>
  <xsl:value-of select="start/@humanReadable"/>
  <xsl:text> - </xsl:text>
  <xsl:value-of select="end/@humanReadable"/>
  <xsl:text>"];
  </xsl:text>
 </xsl:template>

 <xsl:template match="task">
  <xsl:choose>
   <xsl:when test="child::task"> <!-- if this task is a supertask -->
    <xsl:text>subgraph "cluster_</xsl:text>
    <xsl:value-of select="@id"/>
    <xsl:text>" {
    </xsl:text>
    <xsl:text>graph [label="</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>\n</xsl:text>
    <xsl:apply-templates select="taskScenario"/>
    <xsl:text>",bgcolor="</xsl:text>
    <xsl:choose>
     <xsl:when test="not(../self::task)">
      <xsl:text>",bgcolor="gray50</xsl:text>
     </xsl:when>
     <xsl:when test="not(../../self::task)">
      <xsl:text>",bgcolor="gray60</xsl:text>
     </xsl:when>
     <xsl:when test="not(../../../self::task)">
      <xsl:text>",bgcolor="gray70</xsl:text>
     </xsl:when>
     <xsl:when test="not(../../../../self::task)">
      <xsl:text>",bgcolor="gray90</xsl:text>
     </xsl:when>
     <xsl:when test="not(../../../../../self::task)">
      <xsl:text>",bgcolor="gray90</xsl:text>
     </xsl:when>
    </xsl:choose>
    <xsl:text>"];
    </xsl:text>
    <xsl:apply-templates select="task"/>
    <xsl:text>}
    </xsl:text>
   </xsl:when>
   <xsl:otherwise>
    <xsl:text>"</xsl:text>
    <xsl:value-of select="@id"/>
    <xsl:text>"</xsl:text>
    <xsl:text> [fillcolor=white,style=filled,label="</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>\n</xsl:text>
    <xsl:apply-templates select="taskScenario"/>
    <xsl:text>"</xsl:text>
    <xsl:if test="taskScenario/@criticalpath='1'">
      <xsl:text> fontcolor=red, color=red, style="bold,filled"</xsl:text>
    </xsl:if>
    <xsl:text>];</xsl:text>
   </xsl:otherwise>
  </xsl:choose>
 </xsl:template>

 <xsl:template match="depends">
  <xsl:text>"</xsl:text>
  <xsl:if test="key('task',@task)/child::task"> <!-- is a supertask -->
    <xsl:text>cluster_</xsl:text>
  </xsl:if>
  <xsl:value-of select="@task"/>
  <xsl:text>"</xsl:text>
  <xsl:text> -- "</xsl:text>
  <xsl:if test="../child::task"> <!-- is a supertask -->
    <xsl:text>cluster_</xsl:text>
  </xsl:if>
  <xsl:value-of select="../@id"/>
  <xsl:text>" [arrowhead=vee];
  </xsl:text>
 </xsl:template>

 <xsl:template match="taskScenario">
  <xsl:value-of select="@scenarioId"/>
  <xsl:text>: </xsl:text>
  <xsl:value-of select="start/@humanReadable"/>
  <xsl:text> - </xsl:text>
  <xsl:value-of select="end/@humanReadable"/>
  <xsl:text>\n</xsl:text>
 </xsl:template>

</xsl:stylesheet>
