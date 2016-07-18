<?php
/**
 * @file
 * Generate a Marker by Germplasm matrix to display genotypes.
 */

/**
 * The Genotype Matrix is generated as a form. This includes both the filter criteria
 * and the matrix itself. You can alter the matrix using hook_form_alter().
 *
 * NOTE: When altering this form, keep in mind that the submit function will pick up
 *   any new fields and add their values (if not empty) to the URL. The query is done
 *   based on the values in the URL in order to ensure the genotype matrix is bookmarkable
 *   and thus sharable.
 *
 * @param $form
 *   The initialized form array.
 * @param $form_state
 *   An array containing build information such as the organism genus.
 * @return
 *   The completed form array.
 */
function nd_genotypes_matrix_filter_form($form, &$form_state) {
  $genus = $form_state['build_info']['args'][0]['genus'];
  $defaults = $form_state['build_info']['args'][0]['defaults'];
  $total_num_rows = 100;

  //dpm($form_state, 'in form');

  if (!$genus) { drupal_set_message('The link/URL must contain the genus of the organism you would like genotypes for.','error'); return ''; }

  // Form select list options.
  //-------------------------
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

  // Determine the select list for sequence backbone.
  $seq_chr = unserialize(variable_get('nd_genotypes_seq_backbones', NULL));

  // Filter Form.
  //-------------------------
  $form['multiple_stocks'] = array(
    '#type' => 'multiple_organism_stock',
    '#title' => 'Germplasm',
    '#organism_title' => 'Source Species',
    '#organism_options' => $organism_options,
    '#name_title' => 'Germplasm/Stock Name',
    '#added_description' => 'Specify the stock (and species of the stock) you want to display the genotypes of.',
    '#default_value' => (isset($defaults['germ'])) ? $defaults['germ'] : array(),
    '#name_autocomplete_path' => 'tripal_ajax/nd_genotypes/genotyped_germplasm/name_to_id/' . $genus . '/only',
    //'#required' => TRUE,
  );

  $description = 'The range of the genome you would like to display variants from. If you enter just the start or just the end position then all variants before or after that location, respectively, will be displayed.';
  $form['seq_range'] = array(
    '#type' => 'markup',
    '#tree' => TRUE,
    '#prefix' => '<div class="seq-range-element form-item"><label>Genome Range</label>',
    '#suffix' => '<div class="description">' . $description . '</div></div>',
  );

  $form['seq_range']['start'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="start-elements container-inline">',
    '#suffix' => '</div>',
  );

  $form['seq_range']['start']['backbone'] = array(
    '#type' => 'select',
    '#title' => 'From',
    '#empty_option' => '- ' . t('Sequence') . ' -',
    '#options' => $seq_chr,
    '#default_value' => (isset($defaults['seq_range']['start']['backbone'])) ? $defaults['seq_range']['start']['backbone'] : NULL,
  );

  $form['seq_range']['start']['pos'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'Start'),
    '#default_value' => (isset($defaults['seq_range']['start']['pos'])) ? $defaults['seq_range']['start']['pos']:  NULL,
  );

  $form['seq_range']['end'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="end-elements container-inline">',
    '#suffix' => '</div>',
  );

  $form['seq_range']['end']['backbone'] = array(
    '#type' => 'select',
    '#title' => 'to',
    '#empty_option' => '- ' . t('Sequence') . ' -',
    '#options' => $seq_chr,
    '#default_value' => (isset($defaults['seq_range']['end']['backbone'])) ? $defaults['seq_range']['end']['backbone'] : NULL,
  );

  $form['seq_range']['end']['pos'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'End'),
    '#default_value' => (isset($defaults['seq_range']['end']['pos'])) ? $defaults['seq_range']['end']['pos']:  NULL
  );

  $form['variant_name'] = array(
    '#type' => 'textarea',
    '#title' => 'Variant Name(s)',
    '#description' => 'A list of variant names you wish to see genotypes for with one variant per line. Note: Although there is no limit',
    '#rows' => 1,
    '#default_value' => (isset($defaults['variant_name'])) ? implode("\n",$defaults['variant_name']) : '',
  );

/*$form['project_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Project Name',
    '#description' => 'The name of the project you want to restrict genotypes to.',
    //'#autocomplete' @todo this should be an autocomplete.
    '#default_value' => (isset($defaults['project_name'])) ? $defaults['project_name'] : '',
  );*/

  $form['variant_type'] = array(
    '#type' => 'select',
    '#title' => 'Variant Type',
    '#description' => 'The types of variants you would like to see genotypes for (e.g. indels only).',
    '#empty_option' => '- ' . t('Choose One to Filter') . ' -',
    '#options' => unserialize(variable_get('nd_genotypes_variant_types', NULL)),
    '#default_value' => (isset($defaults['variant_type'])) ? $defaults['variant_type'] : NULL,
  );

/*$form['marker_type'] = array(
    '#type' => 'select',
    '#title' => 'Marker Type',
    '#description' => 'The types of markers you would like to see genotypes for (e.g. exome capture).',
    '#empty_option' => '- ' . t('Choose One to Filter') . ' -',
    '#options' => unserialize(variable_get('nd_genotypes_marker_types', NULL)),
    '#default_value' => (isset($defaults['marker_type'])) ? $defaults['marker_type'] : NULL,
  );*/

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Search',
  );

  return $form;
}

/**
 * Validate the options.
 */
function nd_genotypes_matrix_filter_form_validate($form, $form_state) {

  // -- Genome Range
  foreach (array('start','end') as $field) {
    if (!empty($form_state['values']['seq_range'][$field]['pos'])) {
      // If the postition is supplied then the chromosome should be to.
      if (empty($form_state['values']['seq_range'][$field]['backbone'])) {
        form_set_error('seq_range]['.$field.'][pos', 'If you supply a '.$field.' position then you need to tell us what sequence that position is on using the drop-down.');
      }
      // The position should be numeric.
      if (!is_numeric($form_state['values']['seq_range'][$field]['pos']) OR $form_state['values']['seq_range']['start']['pos'] < 0 ) {
        form_set_error('seq_range]['.$field.'][pos', 'The '.$field.' position must be a positive whole number.');
      }
    }
  }
  // The start should be smaller than the end.
  if (!empty($form_state['values']['seq_range']['start']['backbone']) AND !empty($form_state['values']['seq_range']['end']['backbone'])) {
    if ($form_state['values']['seq_range']['start']['backbone'] > $form_state['values']['seq_range']['end']['backbone']) {
      form_set_error('seq_range][end][backbone', 'The end position must be greater than the start position.');
    }
    if ($form_state['values']['seq_range']['start']['backbone'] == $form_state['values']['seq_range']['end']['backbone']
      AND $form_state['values']['seq_range']['start']['pos'] > $form_state['values']['seq_range']['end']['pos']) {
      form_set_error('seq_range][end][pos', 'The end position must be greater than the start position.');
    }
  }

  // -- Variant Names.
  // Warning: if there are commas or tabs, perhaps you messed up the format?
  if (strpos($form_state['values']['variant_name'], ',') !== false) {
    drupal_set_message("We have detected a comma in your variant name filter. Note that to search for more than one variant each name should be on it's own line, not separated by comma's.", 'warning');
  }
  if (strpos($form_state['values']['variant_name'], "\t") !== false) {
    drupal_set_message("We have detected a tab in your variant name filter. Note that to search for more than one variant each name should be on it's own line, not separated by tabs.", 'warning');
  }
}

/**
 * Add the fitlers specified to the path.
 */
function nd_genotypes_matrix_filter_form_submit(&$form, &$form_state) {

  $form_state['rebuild'] = FALSE;

  // Save all the values to go in the query.
  // We do this instead of using form_state directly so that we can unset the
  // values as add them to the query.
  $values = $form_state['values'];

  // Unset non-field values.
  unset($values['submit'], $values['form_build_id'], $values['form_token'], $values['form_id'], $values['op']);

  // Handle germplasm.
  $query = array();
  if (isset($values['multiple_stocks']['existing'])) {
    foreach($values['multiple_stocks']['existing'] as $stock) {
      if (!empty($stock) and !empty($stock['stock_id'])) {
        $query['germplasm'][] = $stock['stock_id'];
      }
    }
    unset($values['multiple_stocks']);
  }

  // Deal with sequence range.
  if (isset($values['seq_range'])) {
    foreach($values['seq_range'] as $k1 => $v1) {
      foreach($v1 as $k2 => $v2) {
        if (!empty($v2)) {
          $query['seq_range'][$k1][$k2] = $v2;
        }
        else {
          unset($query['seq_range'][$k1][$k2]);
        }
      }
    }
    unset($values['seq_range']);
  }

  // Separate the variant_names into multiple arguements.
  // Not only does this keep the URL readable, it also does some processing ahead of time.
  if (isset($values['variant_name']) AND !empty($values['variant_name'])) {
    $list = preg_split("/\s*(\r\n|\n|\r)\s*/", $values['variant_name']);
    $query['variant_name'] = $list;
    unset($values['variant_name']);
  }

  // Add the remaining simple arguements into the URL.
  foreach ($values as $field => $v) {
    if (is_array($v)) {
      foreach ($v as $k => $sv) {
        if (!empty($sv)) {
          $query[$field][$k] = $sv;
        }
        else {
          unset($query[$field][$k]);
        }
      }
    }
    elseif (!empty($v)) {
      $query[$field] = $v;
    }
    else {
      unset($query[$field]);
    }
  }

  $form_state['redirect'] = array('chado/genotype/Lens', array('query' => $query));
}

/**
 * A helper function that determines the link URLs for the pager based on the first
 * and last location in the current matrix.
 *
 * @param $first_loc
 *   The first location shown in the matrix in the form backbone:start (e.g. Chr1:168)
 * @param $last_loc
 *   The last location shown in the matrix in the form backbone:start (e.g. Chr1:832)
 * @param $max_rows
 *   The total number of rows to allow on a single page.
 * @param $num_rows
 *   The total number of rows in the current matrix.
 *
 * @return
 *   An array of URLs. For more information see theme_genotype_matrix_pager() as the
 *   results of this function are expected to be passed directly to that one.
 */
function nd_genotypes_genotype_matrix_get_pager($vars) {

  $pager = array('label' => $vars['first_loc'].'-'.$vars['last_loc']);
  $curr_path = current_path();
  $query_param = drupal_get_query_parameters();

  // Adding the label to give context to the current page.
  $tmp_start = explode(':', $vars['first_loc']);
  $tmp_end = explode(':', $vars['last_loc']);
  if ($tmp_start[0] == $tmp_end[0]) {
    $pager['label'] = $tmp_start[0] . ':' . $tmp_start[1] . '-' . $tmp_end[1];
  }

  // If the pager is not set then we are on the first page...
  // we want to keep track of that ;-).
  $is_first_page = FALSE;
  if (!isset($query_param['pager_loc'])) {
    $query_param['pager_loc']['first'] = $vars['first_loc'];
    $is_first_page = TRUE;
  }
  elseif ($query_param['pager_loc']['current'] == $query_param['pager_loc']['first']) {
    $is_first_page = TRUE;
  }
  // When on a given page we store the current first location in the link to the next page
  // in order to be able to page efficiently. Unfortunatly this only works if you only
  // go back a single page. After that point we now longer have any idea what came before
  // so we have to use a query to determine it.
  elseif (!isset($query_param['pager_loc']['previous']) OR $query_param['pager_loc']['current'] == $query_param['pager_loc']['previous']) {
    $tmp = explode(':', $vars['first_loc']);
    $prev_loc = NULL;

    // In order to ensure the paging is in reference to the current filtered set we need
    // to use the same where criteria that was used to generate the current page.
    // Thus we build our where clauses based on the page query.
    $mode = 'SELECT';
    $query = $vars['query'];
    $query['sql'] = array("SELECT srcfeature_name||':'||fmin as loc");
    foreach ($vars['query']['sql'] as $k => $string) {
      if (preg_match('/FROM/i',$string)) {
        $mode = 'FROM|WHERE';
      } elseif (preg_match('/(GROUP|ORDER|OFFSET|LIMIT)/i',$string)) {
        $mode = 'GROUP|ORDER|OFFSET|LIMIT';
      }

      if ($mode == 'FROM|WHERE') {
        // We need to change the direction of the pager query when looking for the
        // first location on the previous page.
        if (preg_match('/:srcfeature/',$string)) {
          $string = '    AND ((srcfeature_name = :srcfeature AND fmin <= :fmin) OR srcfeature_name < :srcfeature)';
        }
        $query['sql'][] = $string;
      }
    }
    $query['sql'][] = '  GROUP BY srcfeature_name, fmin';
    // We reverse the ordering so that we can travel back 100 records. Unfortunatly
    // a negative offset is not aloud so this is the equivalent.
    $query['sql'][] = '  ORDER BY srcfeature_name DESC, fmin DESC';
    $query['sql'][] = '  OFFSET '.($vars['max_rows']).' LIMIT 1';
    // Then set the pager args.
    $query['args'][':srcfeature'] = $tmp[0];
    $query['args'][':fmin'] = $tmp[1];

    $prev_loc = chado_query(implode(" \n",$query['sql']),
      $query['args'])->fetchField();
    $query_param['pager_loc']['previous'] = $prev_loc;

    // If we were unable to determine the previous then just remove it from the URL.
    if (empty($prev_loc)) unset($query_param['pager_loc']['previous']);
  }

  $query_param['pager_loc']['current'] = $vars['first_loc'];

  // If this is the first page then determine the last page and save it in the url for later.
  if ($is_first_page) {

    // In order to ensure the paging is in reference to the current filtered set we need
    // to use the same where criteria that was used to generate the current page.
    // Thus we build our where clauses based on the page query.
    $mode = 'SELECT';
    $query = $vars['query'];
    $query['sql'] = array("SELECT srcfeature_name||':'||fmin as loc");
    foreach ($vars['query']['sql'] as $k => $string) {
      if (preg_match('/FROM/i',$string)) {
        $mode = 'FROM|WHERE';
      } elseif (preg_match('/(GROUP|ORDER|OFFSET|LIMIT)/i',$string)) {
        $mode = 'GROUP|ORDER|OFFSET|LIMIT';
      }

      if ($mode == 'FROM|WHERE') {
        // We do not want a pager clause when determining the last page
        // but save everything else!
        if (!preg_match('/:srcfeature/',$string)) {
          $query['sql'][] = $string;
        }
      }
    }
    $query['sql'][] = '  GROUP BY srcfeature_name, fmin';
    // We reverse the ordering so that we can travel back 100 records. Unfortunatly
    // a negative offset is not aloud so this is the equivalent.
    $query['sql'][] = '  ORDER BY srcfeature_name DESC, fmin DESC';
    $query['sql'][] = '  OFFSET '.($vars['max_rows']-1).' LIMIT 1';
    // Then remove the pager args.
    unset($query['args'][':srcfeature'], $query['args'][':fmin']);

    $lastest_loc = chado_query(implode(" \n",$query['sql']),
      $query['args'])->fetchField();
    $query_param['pager_loc']['last'] = $lastest_loc;
  }

  $is_last_page = FALSE;
  if ($vars['num_rows'] < $vars['max_rows']) {
    $is_last_page = TRUE;
  }

  // Provide a link back to the first page.
  if (!$is_first_page) {
    $q = $query_param;
    unset($q['pager_loc']);
    $pager['first'] = url($curr_path, array('query' => $q));
  }


  // If we are not on the first page then we want to provide links for first & previous.
  $pager['prev'] = NULL;
  if (!$is_first_page & isset($query_param['pager_loc']['previous'])) {
    $q = $query_param;
    $q['pager_loc']['current'] = $q['pager_loc']['previous'];
    $pager['prev'] = url($curr_path, array('query' => $q));
  }
  elseif (!$is_first_page) {
    $pager['prev'] = $pager['first'];
  }

  // If we are not on the last page, then we want to provide links for next & last.
  $pager['next'] = NULL;
  if (!$is_last_page) {
    $q = $query_param;
    $q['pager_loc']['current'] = $vars['next_page_loc'];
    $q['pager_loc']['previous'] = $query_param['pager_loc']['current'];
    $pager['next'] = url($curr_path, array('query' => $q));
  }

  // Add a last link!
  if (isset($query_param['pager_loc']['last']) AND $query_param['pager_loc']['last'] != $query_param['pager_loc']['current']) {
    $q = $query_param;
    $q['pager_loc']['current'] = $q['pager_loc']['last'];
    unset($q['pager_loc']['previous']);
    $pager['last'] = url($curr_path, array('query' => $q));
  }

  return $pager;
}