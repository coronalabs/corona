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

  <xsl:key
    name="iobjToRemove"
    match="wix:Component[ substring( wix:File/@Source, string-length( wix:File/@Source ) - 4 ) = '.iobj' ]"
    use="@Id"
  />
  <xsl:key
      name="ipdbToRemove"
      match="wix:Component[ substring( wix:File/@Source, string-length( wix:File/@Source ) - 4 ) = '.ipdb' ]"
      use="@Id"
  />
  <xsl:key
      name="pdbToRemove"
      match="wix:Component[ substring( wix:File/@Source, string-length( wix:File/@Source ) - 3 ) = '.pdb' ]"
      use="@Id"
  />

  <xsl:template match="*[ self::wix:Component or self::wix:ComponentRef ][ key( 'iobjToRemove', @Id ) ]" />
  <xsl:template match="*[ self::wix:Component or self::wix:ComponentRef ][ key( 'ipdbToRemove', @Id ) ]" />
  <xsl:template match="*[ self::wix:Component or self::wix:ComponentRef ][ key( 'pdbToRemove', @Id ) ]" />
  
</xsl:stylesheet>
