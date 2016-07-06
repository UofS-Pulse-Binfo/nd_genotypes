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

  // Determine where/args needed to limit the query based on the URL.
  $pager_where = '';
  $pager_args = array();
  if (isset($_GET['pager_loc']['current'])) {

    if ($_GET['pager_loc']['current'] != $_GET['pager_loc']['first']) {
      $parts = explode(':', $_GET['pager_loc']['current']);
      $pager_where = ' AND ((srcfeature_name = :srcfeature AND fmin > :fmin) OR srcfeature_name > :srcfeature) ';
      $pager_args = array(
        'srcfeature' => $parts[0],
        'fmin' => $parts[1],
      );
    }
  }

  // Retrieve the data.
  $args = array_merge($stocks, $pager_args);
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
          WHERE (" . implode(' OR ', $stock_where) . ") " . $pager_where
        ."ORDER BY srcfeature_name ASC, fmin ASC "
        ."LIMIT " . $limit;
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
  $first_loc = NULL;
  foreach($resource as $r) {

    if (!isset($first_loc)) {
      $first_loc = $r->srcfeature_name . ':' .$r->fmin;
    }

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

  // PAGER
  //-------------------------
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

  $output .= theme('genotype_matrix_pager', $pager);

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

function theme_genotype_matrix_pager($vars) {
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
