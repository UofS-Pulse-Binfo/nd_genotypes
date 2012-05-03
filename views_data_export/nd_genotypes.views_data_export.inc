<?php

/**
 * Implementation of hook_views_plugins().
 */
function nd_genotypes_views_plugins() {
  $tc_path = drupal_get_path('module', 'nd_genotypes');

  $style_defaults = array(
    'path' => $tc_path . '/views_data_export/plugins',
    'parent' => 'views_data_export',
    'theme' => 'views_data_export',
    'theme path' => $tc_path . '/views_data_export/theme',
    'theme file' => 'nd_genotypes.views_data_export.inc',
    'uses row plugin' => FALSE,
    'uses fields' => TRUE,
    'uses options' => TRUE,
    'type' => 'data_export',
  );

  
  return array( 
    'style' => array(   
      // add SNP marked-up FASTA format as a new style for the existing views_export_data Display
      'views_data_export_snp_markedup_fasta' => array(
        'title' => t('SNP marked-up FASTA file'),
        'help' => t('Display results in FASTA format which includes [allele1/allele2] markup for the given SNP. All fields in results are on the definition line while the feature.residues field must be present.'),
        'handler' => 'nd_genotypes_plugin_style_export_snp_markedup_fasta',
        // Views Data Export element that will be used to set additional headers when serving the feed.
        'export headers' => array('Content-type: text/plain; charset=utf-8'),
        // Views Data Export element mostly used for creating some additional classes and template names.
        'export feed type' => 'marked-up fasta',
        'export feed text' => 'Marked-up FASTA',
        'export feed file' => '%view.fna',
        'export feed icon' => $tc_path . '/views_data_export/images/fasta.png',
        'additional themes' => array(
          'views_data_export_snp_markedup_fasta_header' => 'style',
          'views_data_export_snp_markedup_fasta_body' => 'style',
          'views_data_export_snp_markedup_fasta_footer' => 'style',
        ),
        'additional themes base' => 'views_data_export_snp_markedup_fasta',
      ) + $style_defaults,      
    ),
  );
}

function template_preprocess_views_data_export_snp_markedup_fasta_body(&$vars) {
   _views_data_export_header_shared_preprocess($vars);
   _views_data_export_body_shared_preprocess($vars);
   $defline_tpl = $vars['options']['defline_fields'];
   
   // iterate through the rows
   foreach ($vars['themed_rows'] as $i => $fields) {
   
      //replace the field tokens with values
      // to generate the definition line
      $defline = $defline_tpl;
      foreach ($fields as $key => $value) {
         $defline = preg_replace("/\[$key\]/",$value,$defline);
      }
      $vars['themed_rows'][$i]['defline'] = $defline;

      // Also cleanup marked-up snp fasta
      // remove html and spaces
      $vars['themed_rows'][$i]['markedup_parent_residues'] = strip_tags( $fields['markedup_parent_residues'] );
      $vars['themed_rows'][$i]['markedup_parent_residues'] = str_replace(' ','',$fields['markedup_parent_residues'] );
   }

}
