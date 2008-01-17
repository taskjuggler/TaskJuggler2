<?xml version="1.0" encoding="UTF-8"?>
<!--
  XSLT stylesheet for converting TaskJuggler XML v2 report to Pert graph in
  Graphviz' dot format

  should be used for instance as follow (with xmlsoft.org's libxslt):
  zcat v2.tjx | xsltproc -novalid tj22pertdot.xsl - | dot -T png > v2.png
  warning: the correct novalid option has two leading dashes (but this can
  not be put in an xml comment such as this one)

  (c) 2007 Grégoire Barbier <gb@gbarbier.org>
  This software is licensed under the terms of the GNU General Public Licence
  version 2, see http://www.gnu.org/licenses/gpl.html
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

 <xsl:output method="text" indent="yes" encoding="UTF-8"/>

 <xsl:template match="/taskjuggler">
  <xsl:text>graph g {
    node[shape=box];
  </xsl:text>
   <xsl:apply-templates select="project"/>
   <xsl:apply-templates select="taskList"/>
   <xsl:apply-templates select="/descendant::depends"/>
   <xsl:apply-templates select="/descendant::precedes"/>
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
  <xsl:text>"</xsl:text>
  <xsl:value-of select="@id"/>
  <xsl:text>"</xsl:text>
  <xsl:text> [fillcolor=white,style=filled,label="</xsl:text>
  <xsl:value-of select="@name"/>
  <xsl:text>\n</xsl:text>
  <xsl:apply-templates select="taskScenario"/>
  <xsl:text>"</xsl:text>
  <xsl:if test="taskScenario/@criticalpath='1'">
   <xsl:text>, fontcolor=red, color=red, style="bold,filled"</xsl:text>
  </xsl:if>
  <xsl:if test="@milestone='1'">
   <xsl:text>, fillcolor="#ffffb0"</xsl:text>
  </xsl:if>
  <xsl:text>];</xsl:text>
  <xsl:if test="../self::task"> <!-- if this task is a subtask -->
   <xsl:text>"</xsl:text>
   <xsl:value-of select="../@id"/>
   <xsl:text>"</xsl:text>
   <xsl:text> -- "</xsl:text>
   <xsl:value-of select="@id"/>
   <xsl:text>" [arrowtail=onormal,style=dashed];
   </xsl:text>
  </xsl:if>
  <xsl:apply-templates select="task"/>
 </xsl:template>

 <xsl:template match="depends">
  <xsl:text>"</xsl:text>
  <xsl:value-of select="@task"/>
  <xsl:text>"</xsl:text>
  <xsl:text> -- "</xsl:text>
  <xsl:value-of select="../@id"/>
  <xsl:text>" [arrowhead=vee];
  </xsl:text>
 </xsl:template>

 <xsl:template match="precedes">
  <xsl:text>"</xsl:text>
  <xsl:value-of select="../@id"/>
  <xsl:text>"</xsl:text>
  <xsl:text> -- "</xsl:text>
  <xsl:value-of select="@task"/>
  <xsl:text>" [arrowhead=vee];
  </xsl:text>
 </xsl:template>


 <xsl:template match="taskScenario">
  <xsl:value-of select="@scenarioId"/>
  <xsl:text>: </xsl:text>
  <xsl:value-of select="start/@humanReadable"/>
  <xsl:choose>
   <xsl:when test="../@milestone='1'"> <!-- if this task is a supertask -->
    <xsl:text> (milestone)</xsl:text>
   </xsl:when>
   <xsl:otherwise>
    <xsl:text> - </xsl:text>
    <xsl:value-of select="end/@humanReadable"/>
   </xsl:otherwise>
  </xsl:choose>
  <xsl:text>\n</xsl:text>
 </xsl:template>

</xsl:stylesheet>
