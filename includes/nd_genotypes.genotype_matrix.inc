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
function nd_genotypes_matrix_filter_form($form, $form_state) {
  $genus = $form_state['build_info']['args'][0];
  $total_num_rows = 100;

  if (!$genus) { drupal_set_message('The link/URL must contain the genus of the organism you would like genotypes for.','error'); return ''; }

  // Add CSS for this form.
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'nd_genotypes') . '/theme/css/nd_genotype_matrix.css',
  );

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

  // Defaults.
  //-------------------------
  // Grab defaults for germplasm from path if form_state is not set.
  $germ_defaults = (isset($form_state['values']['multiple_stocks'])) ? $form_state['values']['multiple_stocks'] : array();
  if (!$germ_defaults AND isset($_GET['germplasm'])) {
    $i = 0;

    // Fill in name/organism if only stock_id is provided.
    foreach ($_GET['germplasm'] as $v) {
      $s = chado_query('SELECT name, organism_id FROM {stock} WHERE stock_id=:id',
        array(':id' => $v))->fetchObject();
      if ($s) {
        $germ_defaults['existing'][$i]['stock_id'] = $v;
        $germ_defaults['existing'][$i]['stock_name'] = $s->name;
        $germ_defaults['existing'][$i]['organism_id'] = $s->organism_id;
      }

      $i++;
    }
  }

  // Grab defaults for the other fields.
  $defaults = array();
  foreach (array('variant_name','project_name','variant_type','marker_type') as $field) {

    // If the field is in the path.
    // We will validate it at the database layer using placeholders.
    if (isset($_GET[$field])) {
      $defaults[$field] = $_GET[$field];
    }
  }

  // Filter Form.
  //-------------------------
  $form['multiple_stocks'] = array(
      '#type' => 'multiple_organism_stock',
      '#title' => 'Genotypes',
      '#organism_title' => 'Source Species',
      '#organism_options' => $organism_options,
      '#name_title' => 'Germplasm/Stock Name',
      '#added_description' => 'Specify the stock (and species of the stock) you want to display the genotypes of.',
      '#default_value' => $germ_defaults,
      '#name_autocomplete_path' => 'tripal_ajax/nd_genotypes/genotyped_germplasm/name_to_id/' . $genus . '/only',
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
    '#default_value' => (isset($_GET['seq_range']['start']['backbone'])) ? $_GET['seq_range']['start']['backbone'] : NULL,
  );

  $form['seq_range']['start']['pos'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'Start'),
    '#default_value' => (isset($_GET['seq_range']['start']['pos'])) ? $_GET['seq_range']['start']['pos']:  NULL,
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
    '#default_value' => (isset($_GET['seq_range']['end']['backbone'])) ? $_GET['seq_range']['end']['backbone'] : NULL,
  );

  $form['seq_range']['end']['pos'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'End'),
    '#default_value' => (isset($_GET['seq_range']['end']['pos'])) ? $_GET['seq_range']['end']['pos']:  NULL
  );

  $form['variant_name'] = array(
    '#type' => 'textarea',
    '#title' => 'Variant Name(s)',
    '#description' => 'A list of variant names you wish to see genotypes for with one variant per line.',
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

  // If there are no germplasm selected then we should not try to render a matrix ;-).
  if (!isset($germ_defaults['existing'])) {
    $form['info'] = array(
      '#type' => 'markup',
      '#markup' => 'Please enter the germplasm you would like to see the genotype of.',
      '#prefix' => '<div class="instructions">',
      '#suffix' => '</div>',
    );
    return $form;
  }

  // The Matrix.
  //-------------------------
  $germplasm = array();

  // The non-germplasm specific header. This will be added to as we process the data.
  $header = array(
    'variant_name' => array('data' => 'Variant Name', 'class' => array('variant_name')),
    'srcfeature_name' => array('data' => 'Backbone', 'class' => array('position','backbone')),
    'fmin' => array('data' => 'Start', 'class' => array('position', 'start')),
    'fmax' => array('data' => 'End', 'class' => array('position','end')),
  );

  // Create a template of what each row will look like. This allows us to initialize
  // an empty row when we first come across a variant and ensure that missing data is handled.
  $template_array = array('variant_name' => '', 'srcfeature_name' => '', 'fmin' => '', 'fmax' => '');

  // Process the germplasm adding them to both the header and the template.
  foreach ($germ_defaults['existing'] as $d) {
    $germplasm[ $d['stock_id'] ] = strtolower(drupal_clean_css_identifier( $d['stock_name'] ));
    $header[ $d['stock_id'] ] = array('data' => $d['stock_name'], 'class' => array('germplasm', $germplasm[ $d['stock_id'] ]));
    $template_array[ $d['stock_id'] ] = '';

    // Assume all germplasm have no calls until we find out otherwise when processin the data.
    $germplasm_no_calls[ $d['stock_id'] ] = $d['stock_name'];
  }

  $form['matrix'] = array(
    '#prefix' => '<div id="genotype-matrix">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => array(),
  );


  // Determine the query & execute it.
  $args = array();
  $sql = nd_genotypes_genotype_matrix_get_query($args, $_GET, array('total_num_rows' => $total_num_rows));
  $resource = chado_query($sql, $args);


  // Generate the table rows.
  // Keep in mind that we retrieve the data as a long listing rather than a table
  // and join it ourself based on variant_id.
  $last_loc = NULL;
  $first_loc = NULL;
  foreach($resource as $r) {

    // Save the first location for use with the pager.
    if (!isset($first_loc)) {
      $first_loc = $r->srcfeature_name . ':' .$r->fmin;
    }

    // Initialize the variant if this is the first time we have come across it.
    if (!isset($form['matrix']['#rows'][$r->variant_id])) {
      // First initialize the row with empty values.
      $form['matrix']['#rows'][$r->variant_id] = $template_array;

      // Then fill in the core information.
      $form['matrix']['#rows'][$r->variant_id]['variant_name'] = array('data' => $r->variant_name, 'class' => array('variant_name'));
      $form['matrix']['#rows'][$r->variant_id]['srcfeature_name'] = array('data' => $r->srcfeature_name, 'class' => array('position','backbone'));
      $form['matrix']['#rows'][$r->variant_id]['fmin'] = array('data' => $r->fmin, 'class' => array('position', 'start'));
      $form['matrix']['#rows'][$r->variant_id]['fmax'] = array('data' => $r->fmax, 'class' => array('position','end'));

    }

    // Since we have data for this germplasm,
    // remove it from the no data list if it's still there.
    if (isset($germplasm_no_calls[$r->germplasm_id])) unset($germplasm_no_calls[$r->germplasm_id]);

    // Determine whether we are still within our row limit.
    $num_rows = sizeof($form['matrix']['#rows']);
    if ($num_rows > $total_num_rows) {
      unset($form['matrix']['#rows'][$r->variant_id]);
      break;
    }

    // Save the last location for use with the pager.
    $last_loc = $r->srcfeature_name . ':' . $r->fmin;

    // Save the call.
    $form['matrix']['#rows'][$r->variant_id][$r->germplasm_id] = array(
      'data' => $r->consensus_allele,
      'class' => array('germplasm', $germplasm[ $r->germplasm_id ], 'genotype', $r->consensus_allele)
    );
  }

  // If there are no results then we should tell the user that...
  if (empty($form['matrix']['#rows'])) {
    unset($form['matrix']);
    $form['no_data'] = array(
      '#type' => 'markup',
      '#markup' => 'No variants matching the above filter criteria had genotypes for the given germplasm.',
      '#prefix' => '<div class="no-results">',
      '#suffix' => '</div>'
    );
    //return $form;
  }

  if (!empty($germplasm_no_calls)) {
    $msg = 'The following ' . sizeof($germplasm_no_calls) . ' germplasm did not have a call under the specified filter criteria: '.implode(', ',$germplasm_no_calls) . '.';
    drupal_set_message($msg, 'warning');
  }

  // Pager.
  //-------------------------
  $form['pager'] = array(
    '#theme' => 'genotype_matrix_pager',
    '#vars' => nd_genotypes_genotype_matrix_get_pager($first_loc, $last_loc, $total_num_rows),
  );

  return $form;
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

  drupal_goto('chado/genotype/Lens', array('query' => $query));
}

/**
 * Determine the query needed to retrieve data for the genotype matrix.
 *
 * @param $args
 *    An array of arguements (most likely empty) to use when executing the query. This is
 *    here as a means to return two parameters to the calling function.
 * @param $get
 *    This should be the parameters in the URL.
 * @param $vars
 *   This includes any additional variables available in the calling function that may be
 *   needed in this one. Includes:
 *    - total_num_rows: the total number of rows we would like in the completed matrix.
 * @return
 *   The SQL query including placeholders mapping to the now updated $args variable.
 *   Expected to be executed using chado_query().
 */
function nd_genotypes_genotype_matrix_get_query(&$args, $get, $vars) {

  // Grab the filter criteria from the path.
  $stocks = array();
  $stock_where = array();
  if (isset($get['germplasm'])) {
    $stocks = array(':germplasm' => $get['germplasm']);
  }

  $limit = sizeof($stocks[':germplasm']) * $vars['total_num_rows'];

  // If there are no stocks we need to warn the user to add some.
  if (empty($stocks)) {
    $output .= '<p>You must enter <em>at least one stock</em> to display genotypes for under <em>Genotypes</em> above.</p>';
    return $output;
  }

  // Determine where/args needed to limit the query based on the URL.
  $pager_where = '';
  $pager_args = array();
  if (isset($get['pager_loc']['current'])) {

    if ($get['pager_loc']['current'] != $get['pager_loc']['first']) {
      $parts = explode(':', $get['pager_loc']['current']);
      $pager_where = ' AND ((srcfeature_name = :srcfeature AND fmin > :fmin) OR srcfeature_name > :srcfeature) ';
      $pager_args = array(
        'srcfeature' => $parts[0],
        'fmin' => $parts[1],
      );
    }
  }

  // Use any other filter parameters in the query using placeholders b/c this is from the URL.
  $joins = '';
  $adtl_where = array();
  $adtl_args = array();
  // -- sequence range.
  if (isset($get['seq_range'])) {
    if (isset($get['seq_range']['start']['backbone'])) {
      if (isset($get['seq_range']['start']['pos'])) {
        $adtl_where[] = '((srcfeature_name = :sbackbone AND fmin >= :sfmin::int) OR srcfeature_name > :sbackbone)';
        $adtl_args[':sbackbone'] = $get['seq_range']['start']['backbone'];
        $adtl_args[':sfmin'] = (int) $get['seq_range']['start']['pos'];
      }
      else {
        $adtl_where[] = '(srcfeature_name = :sbackbone OR srcfeature_name > :sbackbone)';
        $adtl_args[':sbackbone'] = $get['seq_range']['start']['backbone'];
      }
    }
    if (isset($get['seq_range']['end']['backbone'])) {
      if (isset($get['seq_range']['end']['pos'])) {
        $adtl_where[] = '((srcfeature_name = :ebackbone AND fmin <= :efmin::int) OR srcfeature_name < :ebackbone)';
        $adtl_args[':ebackbone'] = $get['seq_range']['end']['backbone'];
        $adtl_args[':efmin'] = (int) $get['seq_range']['end']['pos'];
      }
      else {
        $adtl_where[] = '(srcfeature_name < :ebackbone OR srcfeature_name = :ebackbone)';
        $adtl_args[':ebackbone'] = $get['seq_range']['end']['backbone'];
      }
    }
  }
  // -- variant name.
  if (isset($get['variant_name']) AND !empty($get['variant_name'])) {
    $adtl_where[] = 'variant_name IN (:names)';
    $adtl_args[':names'] = $get['variant_name'];
  }
  // -- variant type.
  if (isset($get['variant_type']) AND !empty($get['variant_type'])) {
    $adtl_where[] = 'variant_id IN (SELECT feature_id FROM {feature} WHERE type_id=:variant_type_id)';
    $adtl_args[':variant_type_id'] = $get['variant_type'];
  }
  // -- marker type. (also applies to filtering by project).
  // This is much more complicated then previously thought. We cannot simply return the
  // consensus call computed across all data for positions with at least one call from
  // the specified technology because that is very misleading. Instead we need a way to
  // compute a separate consensus call per technology in addition to the master consensus
  // call. This allows us to simply choose the consensus call for a given technology
  // when asked for it.
  // One way to pull out such a consensus might be to use the new PostgreSQL 9.4
  // FILTER functionality which allows you to filter an aggregated set in place.
  // Unfortunatly this doesn't extend well to an array per technology since you
  // need to hard-code the where clause.
  /* @code
  SELECT gc.variant_id, array_agg(g.description) as alleles,
    array_agg(g.description) FILTER(WHERE stock_id=82) stock_82
  FROM genotype_call gc
  LEFT JOIN genotype g ON g.genotype_id=gc.genotype_id
  GROUP BY gc.variant_id LIMIT 5;
 variant_id |             alleles             | stock_82
------------+---------------------------------+----------
    1094657 | {CC,CC,CC,GG,GG,CC,CG,CC,CC,CC} | {CC}
    1094659 | {TT,TT,TC,TT,CC,TC,TC,TT,CC,CC} | {CC}
    1094661 | {AA,GG,AA,AG,AA,AA,AA,AA,AA,AA} | {AA}
    1094663 | {TT,AT,AT,AT,AA,TT,AA,AA,AA,AA} | {AA}
    1094665 | {AG,GG,AA,AG,AA,AG,AA,GG,AA,GG} | {AA}
  */
  //if (isset($get['marker_type']) AND !empty($get['marker_type'])) {
    //$adtl_where[] = 'markers::bigint[] <@ (SELECT array_agg(feature_id) FROM {featureprop} WHERE type_id=2534 AND value=:marker_type GROUP BY type_id)';
    //$joins = ' LEFT JOIN chado.featureprop ON ARRAY[featureprop.feature_id] <@ genotype_mview.markers::bigint[]';
    //$adtl_where[] = '(SELECT value from {featureprop} WHERE feature_id = ANY(genotype_mview.markers::bigint[]))=:marker_type';
    //$adtl_args[':marker_type'] = $get['marker_type'];
  //}

  $adtl_where = (!empty($adtl_where)) ? ' AND ' . implode(' AND ', $adtl_where) : '';

  // Retrieve the data.
  $args = array_merge($stocks, $pager_args, $adtl_args);
  $sql = "SELECT
            variant_id,
            variant_name,
            srcfeature_name,
            fmin,
            fmax,
            germplasm_id,
            germplasm_name,
            consensus_allele
          FROM {genotype_mview} "
       .  $joins
       ." WHERE germplasm_id IN (:germplasm) " . $pager_where . $adtl_where
       ." ORDER BY srcfeature_name ASC, fmin ASC "
       ." LIMIT " . $limit;

  return $sql;
}

/**
 * A helper function that determines the link URLs for the pager based on the first
 * and last location in the current matrix.
 *
 * @param $first_loc
 *   The first location shown in the matrix in the form backbone:start (e.g. Chr1:168)
 * @param $last_loc
 *   The last location shown in the matrix in the form backbone:start (e.g. Chr1:832)
 * @param $total_num_rows
 *   The total number of rows in the current matrix.
 *
 * @return
 *   An array of URLs. For more information see theme_genotype_matrix_pager() as the
 *   results of this function are expected to be passed directly to that one.
 */
function nd_genotypes_genotype_matrix_get_pager($first_loc, $last_loc, $total_num_rows) {

  $pager = array('label' => $first_loc.'-'.$last_loc);
  $curr_path = current_path();
  $query_param = drupal_get_query_parameters();

  // Adding the label to give context to the current page.
  $tmp_start = explode(':', $first_loc);
  $tmp_end = explode(':', $last_loc);
  if ($tmp_start[0] == $tmp_end[0]) {
    $pager['label'] = $tmp_start[0] . ':' . $tmp_start[1] . '-' . $tmp_end[1];
  }

  // If the pager is not set then we are on the first page...
  // we want to keep track of that ;-).
  $is_first_page = FALSE;
  $is_last_page = FALSE;
  if (!isset($query_param['pager_loc'])) {
    $query_param['pager_loc']['first'] = $first_loc;
    $is_first_page = TRUE;
  }
  elseif ($query_param['pager_loc']['current'] == $query_param['pager_loc']['first']) {
    $is_first_page = TRUE;
  }
  // For cases where they've gone back a page already, we won't know what the
  // previous page is since this is added to the url when they click the next button.
  // While the url method is used for performance reasons, it makes sense to query to
  // determine the previous page in the rare case that users use the previous button.
  // In most cases when the user wants to go back they'll jump back to the first page
  // which doesn't cause a performance hit.
  elseif (!isset($query_param['pager_loc']['previous']) OR $query_param['pager_loc']['current'] == $query_param['pager_loc']['previous']) {
    $tmp = explode(':', $first_loc);
    $prev_loc = chado_query("
      SELECT srcfeature_name||':'||fmin
      FROM {genotype_mview}
      WHERE ((srcfeature_name = :srcfeature AND fmin < :min) OR srcfeature_name < :srcfeature)
      GROUP BY srcfeature_name, fmin
      ORDER BY srcfeature_name DESC, fmin DESC
      OFFSET ".($total_num_rows)." LIMIT 1",
      array(':srcfeature' => $tmp[0], ':min' => $tmp[1]))->fetchField();
    $query_param['pager_loc']['previous'] = $prev_loc;

    // If we were unable to determine the previous then just remove it from the URL.
    if (empty($prev_loc)) unset($query_param['pager_loc']['previous']);
  }

  $query_param['pager_loc']['current'] = $first_loc;


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
    $q['pager_loc']['current'] = $last_loc;
    $q['pager_loc']['previous'] = $query_param['pager_loc']['current'];
    $pager['next'] = url($curr_path, array('query' => $q));
  }

  return $pager;
}

/**
 * Theme the pager.
 */
function theme_genotype_matrix_pager($vars) {
  $element = $vars['element'];
  $vars = $element['#vars'];
  $output = '';

  $output .= '<div class="item-list">
      <ul class="pager">
        <li class="pager-previous">';

  if ($vars['prev']) {
    $output .= '<a href="'.$vars['prev'].'">‹ previous</a>';
  }
  else {
    $output .= '‹ previous';
  }

  $output .= '</li>
        <li class="pager-current">'.$vars['label'].'</li>
        <li class="pager-next">';

  if ($vars['next']) {
    $output .= '<a href="'.$vars['next'].'">next ›</a>';
  }
  else {
    $output .= 'next ›';
  }

  $output .= '</li>
      </ul>
    </div>';

  if (isset($vars['first'])) {
    $output .= '<span class="gotofirst"><a href="'.$vars['first'].'">Go to First Page</a></span>';
  }

  return $output;
}
