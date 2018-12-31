<?xml version="1.0" ?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:wix="http://schemas.microsoft.com/wix/2006/wi">

  <!-- Copy all attributes and elements to the output. -->
  <xsl:template match="@*|*">
    <xsl:copy>
      <xsl:apply-templates select="@*|*" />
    </xsl:copy>
  </xsl:template>
  <xsl:output method="xml" indent="yes" />

  <xsl:key name="file-search" match="wix:Component[wix:File/@Source='$(var.CoronaSdkDir)\Corona Simulator.pdb']" use="@Id"/>

  <!-- Remove directories. -->
  <xsl:template match="wix:Component[wix:File/@Source='$(var.CoronaSdkDir)\Corona Simulator.pdb']" />

  <!-- Remove componentsrefs referencing components in those directories. -->
  <xsl:template match="wix:ComponentRef[key('file-search', @Id)]" />
</xsl:stylesheet>
