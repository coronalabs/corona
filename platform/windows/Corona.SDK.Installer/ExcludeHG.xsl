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

  <!-- Search directories for the components that will be removed. -->
  <xsl:key name="hg-search" match="wix:Directory[@Name = '.hg']" use="descendant::wix:Component/@Id" />

  <!-- Remove directories. -->
  <xsl:template match="wix:Directory[@Name='.hg']" />

  <!-- Remove componentsrefs referencing components in those directories. -->
  <xsl:template match="wix:ComponentRef[key('hg-search', @Id)]" />
</xsl:stylesheet>
