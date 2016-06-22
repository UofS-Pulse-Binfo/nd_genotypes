<?php
/**
 * @file
 * Generate a Marker by Germplasm matrix to display genotypes.
 */

/**
 * Render the genotype matrix page.
 *
 * This page will have a filter form embedded in it as well as displaying the
 * genotype matrix table.
 *
 * @param $genus
 *   The genus of the organism germplasm should belong to.
 */
function nd_genotypes_matrix_page($genus) {
  $output = '';
  $total_num_rows = 100;

  if (!$genus) { drupal_set_message('The link/URL must contain the genus of the organism you would like genotypes for.','error'); return ''; }

  // Set the title & perhaps the breadcrumb accordingly.
  drupal_set_title(ucfirst($genus) . ' Genotypes');

  // Add CSS.
  drupal_add_css(drupal_get_path('module','nd_genotypes').'/theme/nd_genotype_matrix.css');

  // Filter Form.
  $form = drupal_get_form('nd_genotypes_matrix_filter_form', $genus);
  $output .= drupal_render($form);

  return $output;
}

/**
 * The Filter form for the genotype matrix page.
 */
function nd_genotypes_matrix_filter_form($form, $form_state) {
  $genus = $form_state['build_info']['args'][0];

  // Determine the select box for species.
  $options = array('order_by' => array(
    'organism_id' => 'ASC'
  ));
  $organism = chado_select_record('organism',array('organism_id', 'genus', 'species'),array('genus' => $genus),$options);
  if (is_array($organism)) {
    foreach ($organism as $o) {
      $organism_genus = $o->genus;
      $organism_options[$o->organism_id] = $o->genus .' '. $o->species;
    }
  }

  // Grab defaults from path if form_state is not set.
  $defaults = (isset($form_state['values']['multiple_stocks'])) ? $form_state['values']['multiple_stocks'] : array();
  if (!$defaults AND isset($_GET['germplasm'])) {
    $i = 0;

    // Fill in name/organism if only stock_id is provided.
    foreach ($_GET['germplasm'] as $v) {
      $s = chado_query('SELECT name, organism_id FROM {stock} WHERE stock_id=:id',
        array(':id' => $v))->fetchObject();
      if ($s) {
        $defaults['existing'][$i]['stock_name'] = $s->name;
        $defaults['existing'][$i]['organism_id'] = $s->organism_id;
      }

      $i++;
    }
  }

  $form['multiple_stocks'] = array(
      '#type' => 'multiple_organism_stock',
      '#title' => 'Genotypes',
      '#organism_title' => 'Source Species',
      '#organism_options' => $organism_options,
      '#name_title' => 'Germplasm/Stock Name',
      '#added_description' => 'Specify the stock (and species of the stock) you want to display the genotypes of.',
			'#default_value' => $defaults,
			'#name_autocomplete_path' => 'tripal_ajax/nd_genotypes/genotyped_germplasm/name_to_id/' . $genus . '/only',
	);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Search',
  );

  return $form;
}

/**
 * Add the fitlers specified to the path.
 */
function nd_genotypes_matrix_filter_form_submit(&$form, &$form_state) {

  $form_state['rebuild'] = FALSE;

  $query = array();
  if (isset($form_state['values']['multiple_stocks']['existing'])) {
    foreach($form_state['values']['multiple_stocks']['existing'] as $stock) {
      if (!empty($stock) and !empty($stock['stock_id'])) {
        $query['germplasm'][] = $stock['stock_id'];
      }
    }
  }


  drupal_goto('chado/genotype/Lens', array('query' => $query));
}
