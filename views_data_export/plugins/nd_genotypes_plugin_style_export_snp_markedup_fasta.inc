<?php
/**
 * @file
 * Plugin include file for export style plugin.
 * This plugin exports a SNP marked-up FASTA file for any view with a column of 
 * locus or marker feature_ids
 */

/**
 * SNP marked-up FASTA style plugin for export plugins.
 *
 * This plugin exports a SNP marked-up FASTA file for any view with a column of 
 * locus or marker feature_ids
 *
 * @ingroup views_style_plugins
 */
class nd_genotypes_plugin_style_export_snp_markedup_fasta extends views_data_export_plugin_style_export {

  /**
   * Set options fields and default values.
   *
   * @return
   * An array of options information.
   */
  function option_definition() {
    $options = parent::option_definition();

  
    return $options;
  }

  /**
   * Options form mini callback.
   *
   * @param $form
   * Form array to add additional fields to.
   * @param $form_state
   * State of the form.
   * @return
   * None.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['descrip'] = array(
      '#type' => 'item',
      '#value' => 'Requirements:<ul>'
        .'<li>There should be a "Chado Feature: Marked-up Parent Residues" field in this view</li>'
        .'<li>Each row in the query should correspond to a single feature</li>'
        .'<li>Each feature should have known genotypes (ie: feature.feature_id => '
          .'feature_genotype.feature_id => feature_genotype.genotype_id => genotype.description)</li>'
        .'<li>Each feature should be located on another feature with known residues'
          .'via the featureloc table.</li>'
        .'</ul>',
      '#weight' => -10
    );
    
    $form['defline_fields'] = array(
      '#prefix' => '<div><div id="edit-options-newline-replacement">',
      '#suffix' => '</div></div>',
      '#type' => 'textfield',
      '#title' => t('Definition line fields'),
      '#required' => TRUE,
      '#default_value' => $this->options['defline_fields'],
      '#description' => t("This field controls the information present in the definition ".
         "line for each sequence. Any of the fields in the view can be used in the ".
         "definition line.  To discover these fields, add the feature residues as the ".
         "last field in the view.  Then edit the field and click the checkbox \"Rewrite ".
         "the output of this field\".  In the replacement patterns section are the ".
         "field replacement tokens that can be used in this field. You may include any ".
         "additional text to separate or qualify these fields.  Only fields that have not ".
         "been excluded from the display will appear."),
    );
    
  }
  
  function query () {
    parent::query();

    if (!isset($this->view->field['markedup_parent_residues'])) {
      drupal_set_message('A "Chado Feature: Marked-up Parent Residues" field needs to be added to the Views Data Export View','error');
    }
  }
}
