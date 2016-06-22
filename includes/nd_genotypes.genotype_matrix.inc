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
  drupal_add_css(drupal_get_path('module','nd_genotypes').'/theme/css/nd_genotype_matrix.css');

  // Filter Form.
  $form = drupal_get_form('nd_genotypes_matrix_filter_form', $genus);
  $output .= drupal_render($form);

  // Grab the filter criteria from the path.
  $stocks = array();
  $stock_where = array();
  if (isset($_GET['germplasm'])) {
    $i = 0;
    foreach ($_GET['germplasm'] as $stock_id) {
      $stocks[':s'.$i] = $stock_id;
      $stock_where[] = 'germplasm_id=:s'.$i;
      $i++;
    }
  }

  $limit = sizeof($stocks) * $total_num_rows;

  // If there are no stocks we need to warn the user to add some.
  if (empty($stocks)) {
    $output .= '<p>You must enter <em>at least one stock</em> to display genotypes for under <em>Genotypes</em> above.</p>';
    return $output;
  }

  // Retrieve the data.
  $args = $stocks;
  $sql = "SELECT
            variant_id,
            variant_name,
            srcfeature_name,
            fmin,
            fmax,
            germplasm_id,
            germplasm_name,
            consensus_allele
          FROM {genotype_mview}
          WHERE (" . implode(' OR ', $stock_where) . ")
          ORDER BY srcfeature_name ASC, fmin ASC
          LIMIT " . $limit;
  $resource = chado_query($sql, $args);

  // Set the non-germplasm specific header.
  $header = array(
    'variant_name' => array('data' => 'Variant Name', 'class' => array('variant_name')),
    'srcfeature_name' => array('data' => 'Backbone', 'class' => array('position','backbone')),
    'fmin' => array('data' => 'Start', 'class' => array('position', 'start')),
    'fmax' => array('data' => 'End', 'class' => array('position','end')),
  );
  $germplasm = array();

  // Generate the table rows.
  // Keep in mind that we retrieve the dat as a long listing rather than a table
  // and join it ourself based on variant_id.
  $rows = array();
  $last_loc = NULL;
  foreach($resource as $r) {

    // Initialize the variant if this is the first time we have come across it.
    if (!isset($rows[$r->variant_id])) {
      $rows[$r->variant_id]['variant_name'] = array('data' => $r->variant_name, 'class' => array('variant_name'));
      $rows[$r->variant_id]['srcfeature_name'] = array('data' => $r->srcfeature_name, 'class' => array('position','backbone'));
      $rows[$r->variant_id]['fmin'] = array('data' => $r->fmin, 'class' => array('position', 'start'));
      $rows[$r->variant_id]['fmax'] = array('data' => $r->fmax, 'class' => array('position','end'));

      $rows[$r->variant_id][$r->germplasm_id] = array();
    }

    // Add Germplasm name to the header.
    $germplasm[$r->germplasm_id] = strtolower(drupal_clean_css_identifier($r->germplasm_name));
    $header[$r->germplasm_id] = array('data' => $r->germplasm_name, 'class' => array('germplasm', $germplasm[$r->germplasm_id]));

    // Determine whether we are still within our row limit.
    $num_rows = sizeof($rows);
    //dpm($num_rows . ' of ' . $total_num_rows);
    if ($num_rows > $total_num_rows) {
      unset($rows[$r->variant_id]);
      break;
    }

    $last_loc = $r->srcfeature_name . ':' . $r->fmin;

    // Save the call.
    $rows[$r->variant_id][$r->germplasm_id] = $r->consensus_allele;

  }

  // Sanitize the table to ensure there is a  cell for each germplasm.
  foreach ($rows as $k => $row) {
    foreach (array_keys($header) as $h) {
      if (isset($germplasm[$h])) {
        if (!isset($rows[$k][$h])) {
          $rows[$k][$h] = array('data' => '', 'class' => array('genotype', $germplasm[$h]));
        }
        elseif (!is_array($rows[$k][$h])) {
          $rows[$k][$h] = array('data' => $rows[$k][$h], 'class' => array('germplasm', $germplasm[$h], 'genotype', $rows[$k][$h]));
        }
      }
    }
  }

  if (sizeof($germplasm) != sizeof($stocks)) {
    drupal_set_message('At least one of the germplasm indicated do not have any genotype calls under the specified criteria.','warning');
  }

  $table = array(
    'header' => $header,
    'rows' => $rows,
  );
  $output .= theme('table', $table);

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
