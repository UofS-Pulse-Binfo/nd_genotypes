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
  $partition = $form_state['build_info']['args'][0]['partition'];
  $total_num_rows = 100;

  if (!$genus) { drupal_set_message('The link/URL must contain the genus of the organism you would like genotypes for.','error'); return ''; }

  $partition = strtolower($partition);

  drupal_set_title("$genus Genotypes");

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

  // Filter Form.
  //-------------------------
  // STAGE #1
  $form['s1'] = array(
    '#type' => 'fieldset',
    '#title' => 'Choose germplasm you are interested in.',
    '#description' => 'Simply enter the name of one germplasm (e.g. CDC Redberry) of interest below and then click the green plus (+) button. You can enter any number of germplasm you are interested in and each will be added to the matrix as they are entered.',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<span class="stages stage1">1</span>',
  );

  $form['s1']['multiple_stocks'] = array(
    '#type' => 'multiple_organism_stock',
    '#title' => 'Germplasm',
    '#organism_title' => 'Source Species',
    '#organism_options' => $organism_options,
    '#name_title' => 'Germplasm/Stock Name',
    '#added_description' => 'Specify the stock (and species of the stock) you want to display the genotypes of.',
    '#default_value' => (isset($defaults['germ'])) ? $defaults['germ'] : array(),
    '#name_autocomplete_path' => 'tripal_ajax/nd_genotypes/genotyped_germplasm/name_to_id/' . $partition . '/only',
    '#missing_stock_error_callback' => 'nd_genotype_matrix_missing_stock_error_callback',
    //'#required' => TRUE,
  );

  if (!isset($defaults['germlist'])) $defaults['germlist'] = array();
  $form['s1']['polymorphic'] = array(
    '#type' => 'pairwise_compare_germplasm',
    '#title' => 'Polymorphic Variants',
    '#options' => array(
      'first' => $defaults['germlist'],
      'second' => $defaults['germlist']
    ),
    '#default_value' => (isset($defaults['polymorphic'])) ? $defaults['polymorphic'] : NULL,
    '#description' => 'Restrict the variants to those that have different allele calls for the selected germplasm.',
  );
  if (sizeof($defaults['germlist']) == 0) $form['s1']['polymorphic']['#disabled'] = TRUE;

  // STAGE #2
  $form['s2'] = array(
    '#type' => 'fieldset',
    '#title' => 'Restrict to the region of the genome. (optional)',
    '#description' => 'If applicable, we recommend you filter to a given region of the genome to make the genotype set more managable.',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#prefix' => '<span class="stages stage2">2</span>',
  );

  $description = 'The range of the genome you would like to display variants from. If you enter just the start or just the end position then all variants before or after that location, respectively, will be displayed.';
  $form['s2']['seq_range'] = array(
    '#type' => 'markup',
    '#tree' => TRUE,
    '#prefix' => '<div class="seq-range-element form-item"><label>Genome Range</label>',
    '#suffix' => '<div class="description">' . $description . '</div></div>',
  );

  $form['s2']['seq_range']['start'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="start-elements">',
    '#suffix' => '</div>',
  );

  $form['s2']['seq_range']['start']['label'] = array(
    '#type' => 'item',
    '#markup' => 'From',
  );

  $form['s2']['seq_range']['start']['backbone'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'Sequence Name'),
    '#autocomplete_path' => 'tripal_ajax/nd_genotypes/genotyped_backbones/name_to_id/' . $partition . '/only',
    '#default_value' => (isset($defaults['seq_range']['start']['backbone'])) ? $defaults['seq_range']['start']['backbone'] : NULL,
  );

  $form['s2']['seq_range']['start']['pos'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'Start Position'),
    '#default_value' => (isset($defaults['seq_range']['start']['pos'])) ? $defaults['seq_range']['start']['pos']:  NULL,
  );

  $form['s2']['seq_range']['end'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="end-elements">',
    '#suffix' => '</div>',
  );

  $form['s2']['seq_range']['end']['label'] = array(
    '#type' => 'item',
    '#markup' => 'to',
  );

  $form['s2']['seq_range']['end']['backbone'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'Sequence Name'),
    '#autocomplete_path' => 'tripal_ajax/nd_genotypes/genotyped_backbones/name_to_id/' . $partition . '/only',
    '#default_value' => (isset($defaults['seq_range']['end']['backbone'])) ? $defaults['seq_range']['end']['backbone'] : NULL,
  );

  $form['s2']['seq_range']['end']['pos'] = array(
    '#type' => 'textfield',
    '#attributes' => array('placeholder' => 'End Position'),
    '#default_value' => (isset($defaults['seq_range']['end']['pos'])) ? $defaults['seq_range']['end']['pos']:  NULL
  );

  // STAGE #3
  $form['s3'] = array(
    '#type' => 'fieldset',
    '#title' => 'Additional Filter criteria. (optional)',
    '#description' => 'We recommend you fill out as many of the following optional filters as possible to narrow the genotype set to those you are most interested in.',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#prefix' => '<span class="stages stage3">3</span>',
  );

  $form['s3']['variant_name'] = array(
    '#type' => 'textarea',
    '#title' => 'Variant Name(s)',
    '#description' => 'A list of variant names you wish to see genotypes for with one variant per line.',
    '#rows' => 1,
    '#default_value' => (isset($defaults['variant_name'])) ? implode("\n",$defaults['variant_name']) : '',
  );

  $project_name = '';
  if (isset($defaults['project_id'])) {
    if (is_array($defaults['project_id'])) {
      $project_id = $defaults['project_id'][0];
    }
    else {
      $project_id = $defaults['project_id'];
    }
    $project_name = chado_query('SELECT name FROM {project} WHERE project_id=:id',
      array(':id' => $project_id))->fetchField();
  }
  $form['s3']['project_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Project Name',
    '#description' => 'The name of the project you want to restrict genotypes to.',
    '#autocomplete_path' => 'tripal_ajax/nd_genotypes/project/name',
    '#default_value' => $project_name,
  );

  $form['s3']['variant_type'] = array(
    '#type' => 'select',
    '#title' => 'Variant Type',
    '#description' => 'The types of variants you would like to see genotypes for (e.g. indels only).',
    '#empty_option' => '- ' . t('Choose One to Filter') . ' -',
    '#options' => unserialize(variable_get('nd_genotypes_'.$partition.'_variant_types', 'a:0:{}')),
    '#default_value' => (isset($defaults['variant_type'])) ? $defaults['variant_type'] : NULL,
  );

  $form['s3']['marker_type'] = array(
    '#type' => 'select',
    '#title' => 'Marker Type',
    '#description' => 'The types of markers you would like to see genotypes for (e.g. exome capture).',
    '#empty_option' => '- ' . t('Choose One to Filter') . ' -',
    '#options' => unserialize(variable_get('nd_genotypes_'.$partition.'_marker_types', NULL)),
    '#default_value' => (isset($defaults['marker_type'])) ? $defaults['marker_type'] : NULL,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Search',
  );

  return $form;
}

/**
 * Validate the options.
 */
function nd_genotypes_matrix_filter_form_validate($form, &$form_state) {

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
      AND !empty($form_state['values']['seq_range']['end']['pos'])
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

  // Check project exists.
  if (isset($form_state['values']['project_name']) AND !empty($form_state['values']['project_name'])) {
    $project_id = chado_query('SELECT project_id FROM {project} WHERE name=:name',
       array(':name' => $form_state['values']['project_name']))->fetchField();
    if (!$project_id) {
      form_set_error('project_name', 'Project "<em>'.$form_state['values']['project_name'].'</em>" doesn\'t exist.');
    }
    else {
      $form_state['values']['project_id'] = array($project_id);
    }
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
  unset($values['submit'], $values['form_build_id'], $values['form_token'], $values['form_id'], $values['op'], $values['add']);

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
    $values['variant_name'] = trim($values['variant_name']);
    $list = preg_split("/\s*(\r\n|\n|\r)\s*/", $values['variant_name']);
    $query['sort'] = 'variant_name';

    // Check if this list already exists.
    $list_id = db_select('ndg_matrix_variant_user_lists','l')
      ->fields('l', array('list_id'))
      ->condition('l.list', json_encode($list), '=')
      ->execute()
      ->fetchField();

    // If this is from an already saved list, don't re-save it!
    if ($list_id) {
      $query['variant_name_list'] = $list_id;
      unset($values['variant_name']);
      // Increase the counter for the number of times this list was used.
      db_query('UPDATE {ndg_matrix_variant_user_lists} SET num_accessed = num_accessed + 1 WHERE list_id = :id AND created != :today',
        array(':id' => $list_id, ':today' => date('Y-m-d')));
    }
    // Now, we should be careful to not store too many variant names in the URL
    // because there is a length limit ;-).
    // Thus, if we have 10 or less, we will use the URL
    elseif (sizeof($list) <= 10) {
      $query['variant_name'] = $list;
      unset($values['variant_name']);
    }
    // Otherwise, we should store the names somewhere else.
    // These names should be stored long term since the url containing their reference
    // can be bookmarked and even published in publications. Thus we decided to store
    // them in a database table. Specifically, public.ndg_matrix_variant_user_lists.
    else {
      $list_id = db_insert('ndg_matrix_variant_user_lists')
        ->fields(array(
          'list' => json_encode($list),
          'num_accessed' => 1,
          'created' => date('Y-m-d'),
        ))
        ->execute();
      $query['variant_name_list'] = $list_id;
      unset($values['variant_name']);
    }
  }

  // Remove project_name if we already have project_id.
  if (isset($values['project_name']) AND isset($values['project_id'])) {
    unset($values['project_name']);
  }

  // Look up subprojects if we have a project entered and the admin has chosen to do so.
  if (isset($values['project_id']) AND variable_get('ndg_follow_project_rels', FALSE)) {
    $type = variable_get('nd_genotypes_subproject_rel_type_id', 0);
    $position = variable_get('nd_genotypes_subproject_rel_position', NULL);
    if ($type AND $position) {
      if ($position == 'subject') {
        $sql = 'SELECT object_project_id FROM {project_relationship} WHERE subject_project_id=:parent_project_id';
      }
      else {
        $sql = 'SELECT subject_project_id FROM {project_relationship} WHERE object_project_id=:parent_project_id';
      }
      $addt_project_ids = chado_query($sql, array(':parent_project_id' => $values['project_id'][0]))->fetchCol();

      // Now add them to the list of projects.
      if (is_array($addt_project_ids)) {
        $values['project_id'] = array_merge($values['project_id'], $addt_project_ids);
      }
    }
  }

  // Remove polymorphic filter if it's empty.
  if ($values['polymorphic']['first'] == '<NONE>' AND $values['polymorphic']['second'] == '<NONE>') {
    unset($values['polymorphic']);
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

  // Initializing paging.
  $query['page'] = 1;

  $form_state['redirect'] = array(current_path(), array('query' => $query));
}

/**
 * This function processes the query parameters for the genotype matrix to determine
 * defaults for the filter form and parameters for the query.
 */
function nd_genotypes_process_query_parameters(&$vars) {

  // Initialize some defaults.
  $vars['query_args'] = array('germplasm_id' => array());
  $vars['germplasm'] = array();
  $vars['defaults'] = array();

  // Start to compile the header and template row.
  $vars['header'] = array(
    'variant_name' => array('data' => 'Variant Name', 'class' => array('variant_name')),
    'srcfeature_name' => array('data' => 'Backbone', 'class' => array('position','backbone')),
    'fmin' => array('data' => 'Start', 'class' => array('position', 'start')),
    'fmax' => array('data' => 'End', 'class' => array('position','end')),
  );
  $vars['template_row'] = array('variant_name' => '', 'srcfeature_name' => '', 'fmin' => '', 'fmax' => '');

  // --Germplasm.
  if (isset($vars['q']['germplasm'])) {
    foreach ($vars['q']['germplasm'] as $germ_id) {

      if (!is_numeric($germ_id)) {
        drupal_set_message('Incorrect format for germplasm_id: <em>'.$germ_id.'</em>. This germplasm has been removed from the search.','error');
        continue;
      }

      // retrieve more details.
      $s = chado_query('SELECT name, organism_id FROM {stock} WHERE stock_id=:id',
        array(':id' => $germ_id))->fetchObject();

      if ($s) {
        // Save details for later use.
        $vars['germplasm'][$germ_id] = array(
          'object' => $s,
          'name' => $s->name,
          'class' => strtolower(drupal_clean_css_identifier( $s->name )),
        );

        // Add to the header and template row.
        $vars['header'][$germ_id] = array('data' => $s->name, 'class' => array('germplasm', $vars['germplasm'][$germ_id]['class']));
        $vars['template_row'][$germ_id] = '';

        // Add to the query.
        $vars['query_args']['germplasm_id'][] = $germ_id;

        // Save as defaults for use in the form.
        // First for the "Germplasm" form element.
        $vars['defaults']['germ']['existing'][] = array(
          'stock_id' => $germ_id,
          'organism_id' => $s->organism_id,
          'stock_name' => $s->name,
        );
        // And second for the polymorphic form element.
        $vars['defaults']['germlist'][$germ_id] = $s->name;
      }
      else {
        drupal_set_message('Unable to find germplasm.', 'error');
      }
    }
  }

  // --Sequence Range.
  if (isset($vars['q']['seq_range'])) {
    if (isset($vars['q']['seq_range']['start']['backbone'])) {
      if (isset($vars['q']['seq_range']['start']['pos'])) {
        $vars['query_args']['start_backbone'] = $vars['q']['seq_range']['start']['backbone'];
        $vars['query_args']['start_pos'] = (int) $vars['q']['seq_range']['start']['pos'];

        $vars['defaults']['seq_range']['start']['backbone'] = $vars['q']['seq_range']['start']['backbone'];
        $vars['defaults']['seq_range']['start']['pos'] = $vars['q']['seq_range']['start']['pos'];
      }
      else {
        $vars['query_args']['start_backbone'] = $vars['q']['seq_range']['start']['backbone'];
        $vars['defaults']['seq_range']['start']['backbone'] = $vars['q']['seq_range']['start']['backbone'];
      }
    }
    if (isset($vars['q']['seq_range']['end']['backbone'])) {
      if (isset($vars['q']['seq_range']['end']['pos'])) {
        $vars['query_args']['end_backbone'] = $vars['q']['seq_range']['end']['backbone'];
        $vars['query_args']['end_pos'] = (int) $vars['q']['seq_range']['end']['pos'];

        $vars['defaults']['seq_range']['end']['backbone'] = $vars['q']['seq_range']['end']['backbone'];
        $vars['defaults']['seq_range']['end']['pos'] = $vars['q']['seq_range']['end']['pos'];
      }
      else {
        $vars['query_args']['end_backbone'] = $vars['q']['seq_range']['end']['backbone'];
        $vars['defaults']['seq_range']['end']['backbone'] = $vars['q']['seq_range']['end']['backbone'];
      }
    }
  }

  // --Variant name.
  if (isset($vars['q']['variant_name']) AND !empty($vars['q']['variant_name'])) {
    $vars['query_args']['variant_name'] = $vars['q']['variant_name'];
    $vars['defaults']['variant_name'] = $vars['q']['variant_name'];
  }
  // Stored as List.
  // If there were more than 10 variant_names entered we will have stored it in the db
  // instead of the URL.
  if (isset($vars['q']['variant_name_list']) AND is_numeric($vars['q']['variant_name_list'])) {
    $list_id = $vars['q']['variant_name_list'];
    $list_json = db_query('SELECT list FROM {ndg_matrix_variant_user_lists} WHERE list_id=:id',
      array(':id' => $list_id))->fetchField();
    $list = json_decode($list_json);
    $vars['query_args']['variant_name_list'] = $list;
    $vars['defaults']['variant_name'] = $list;
  }

  // Set the defaults for all other fields in the form.
  $fields = $vars['q'];
  unset($fields['germplasm'], $fields['seq_range']);
  foreach ($fields as $field => $value) {
    // We will validate it at the database layer using placeholders.
    $vars['defaults'][$field] = $value;
    $vars['query_args'][$field] = $value;
  }
}

/**
 *
 */
function nd_genotypes_retrive_matrix_postgresql(&$vars) {
  $vars['variants'] = array();
  $vars['data'] = array();

  if (!ndg_mview_tables_exist($vars['partition'])) {
    return FALSE;
  }

  //-------------------------
  //  First, grab the variants.

  // Compile all the parameters we will use when grabbing the calls so that we can restrict
  // the variant list to those that will have data once we do retrieve the calls.
  $query_args = $vars['query_args'];
  $call = $variant = $call_args = $variant_args = $variant_joins = array();

  // Sequence Ranges:
  // First get the arguements. There must always be a backbone and fmin even if the user
  // did not provide one.
  if (isset($query_args['start_backbone'])) {
    if (isset($query_args['start_pos'])) {
      $variant_args[':sbackbone'] = $query_args['start_backbone'];
      $variant_args[':sfmin'] = (int) $query_args['start_pos'] -1;
    }
    else {
      $variant_args[':sbackbone'] = $query_args['start_backbone'];
      $variant_args[':sfmin'] = 1;
    }
    unset($query_args['start_backbone'], $query_args['start_pos']);
  }
  if (isset($query_args['end_backbone'])) {
    if (isset($query_args['end_pos'])) {
      $variant_args[':ebackbone'] = $query_args['end_backbone'];
      $variant_args[':efmin'] = (int) $query_args['end_pos'] -1;
    }
    else {
      $variant_args[':ebackbone'] = $query_args['end_backbone'];
      // @TODO: Switch to bigints then change this value to match the largest bigint.
      $variant_args[':efmin'] = 2147483645;
    }
    unset($query_args['end_backbone'], $query_args['end_pos']);
  }
  // Now add the query where condition.
  if (isset($variant_args[':sbackbone']) AND isset($variant_args[':ebackbone'])) {
    $variant[] = 'ROW(v.srcfeature_name, v.fmin) BETWEEN ROW(:sbackbone, :sfmin) AND ROW(:ebackbone,:efmin)';
  }
  elseif (isset($variant_args[':sbackbone'])) {
    $variant[] = 'ROW(v.srcfeature_name, v.fmin) >= ROW(:sbackbone, :sfmin)';
  }
  elseif (isset($variant_args[':ebackbone'])) {
    $variant[] = 'ROW(v.srcfeature_name, v.fmin) <= ROW(:ebackbone, :efmin)';
  }

  // Variant Type:
  if (isset($query_args['variant_type'])) {
    $variant[] = 'variant_type=:variant_type';
    $variant_args[':variant_type'] = $query_args['variant_type'];
    unset($query_args['variant_type']);
  }

  // Variant Name:
  $variant_name_filter = FALSE;
  if (isset($query_args['variant_name'])) {
    $variant[] = "variant_name IN (VALUES('" . implode("'), ('", $query_args['variant_name']) . "'))";
    //$variant_args[':variant_names'] = $query_args['variant_name'];
    unset($query_args['variant_name']);
    $variant_name_filter = TRUE;
  }
  elseif (isset($query_args['variant_name_list'])) {
    $list_json = db_query('SELECT list FROM {ndg_matrix_variant_user_lists} WHERE list_id=:id',
      array(':id' => $query_args['variant_name_list']))->fetchField();
    $list = json_decode($list_json);
    $variant[] = "variant_name IN (VALUES('" . implode("'), ('", $list) . "'))";
  }

  // Polymorphic:
  $polymorphic_filter = FALSE;
  if (isset($query_args['polymorphic'])
    AND is_numeric($query_args['polymorphic']['first'])
    AND is_numeric($query_args['polymorphic']['second'])) {
      if (!$variant_name_filter) {
        $variant_joins[] = "LEFT JOIN (
            SELECT a.variant_id, a.allele_call=b.allele_call as monomorphic
            FROM {mview_ndg_calls} a, {mview_ndg_calls} b
            WHERE a.variant_id=b.variant_id AND a.germplasm_id=:poly1 AND b.germplasm_id=:poly2
          ) p ON p.variant_id=v.variant_id ";
        $variant_args[':poly1'] = $query_args['polymorphic']['first'];
        $variant_args[':poly2'] = $query_args['polymorphic']['second'];
        $variant[] = "p.monomorphic IS FALSE";
      }
      $polymorphic_filter = TRUE;
  }

  // Germplasm:
  if (isset($query_args['germplasm_id']) AND !empty($query_args['germplasm_id'])) {
    $call[] = 'germplasm_id IN (:germplasm_ids)';
    $call_args[':germplasm_ids'] = $query_args['germplasm_id'];
    unset($query_args['germplasm_id']);
  }
  else {
    return FALSE;
  }

  // Project:
  if (isset($query_args['project_id']) AND !empty($query_args['project_id'])) {
    if (sizeof($query_args['project_id']) == 1) {
      $call[] = 'project_id = :project_id';
    }
    else {
      $call[] = 'project_id IN (:project_id)';
    }
    $call_args[':project_id'] = $query_args['project_id'];
  }

  // Marker Type:
  if (isset($query_args['marker_type'])) {
    $call[] = 'marker_type = :marker_type';
    $call_args[':marker_type'] = $query_args['marker_type'];
  }

  // Now determine the query for the variants.
  $entity_type = chado_query("
    SELECT 'bio_data_'||bundle_id FROM chado_bundle
    WHERE data_table='feature'
      AND type_id IN (SELECT cvterm_id FROM chado.cvterm WHERE name='sequence_variant');
  ")->fetchField();
  $sql = "SELECT v.*, cf.entity_id \n"
       . "FROM {mview_ndg_variants} v \n"
       . "  LEFT JOIN [chado_".$entity_type."] cf ON cf.record_id=v.variant_id "
       . implode("\n", $variant_joins)
       . "WHERE \n";

  // Add in conditions on the variants table.
  if ($variant) {
    $sql .= implode("\n  AND ",$variant) . "\n  AND ";
  }

  // Add in conditions on the call table.
  $sql .= "EXISTS ( \n"
       . "    SELECT true \n"
       . "    FROM {mview_ndg_calls} call \n"
       . "    WHERE \n"
       . "      " . implode("\n      AND ", $call) .  "\n"
       . "      AND call.variant_id=v.variant_id\n"
       . "  ) \n";

  if ($polymorphic_filter AND $variant_name_filter) {
    $sql = "SELECT * FROM (\n"
      . $sql . ") v \n"
      . "LEFT JOIN (
            SELECT a.variant_id, a.allele_call=b.allele_call as monomorphic
            FROM {mview_ndg_calls} a, {mview_ndg_calls} b
            WHERE a.variant_id=b.variant_id AND a.germplasm_id=:poly1 AND b.germplasm_id=:poly2
          ) p ON p.variant_id=v.variant_id \n"
      . "WHERE p.monomorphic IS FALSE \n";
      $variant_args[':poly1'] = $query_args['polymorphic']['first'];
      $variant_args[':poly2'] = $query_args['polymorphic']['second'];
  }

  // Order and Limit.
  // First determine sort columns.
  if (isset($query_args['sort']) AND $query_args['sort'] == 'variant_name') {
    $order = array('variant_name', 'srcfeature_name', 'fmin');
  }
  else {
    $order = array('srcfeature_name', 'fmin');
  }
  // Then add in the sort order and limit.
  if (isset($query_args['page']) AND is_numeric($query_args['page']) AND $query_args['page'] != 1) {
    // Normal pages (ie: 1, 2, 3, etc).
    if ($query_args['page'] > 1 ) {
      // For example, "\nORDER BY srcfeature_name ASC, fmin ASC \n"
      $sql .= "\nORDER BY " . implode(' ASC, ', $order) . " ASC \n";
      $sql .= "OFFSET " . (100 * ($query_args['page'] - 1)) . " LIMIT 100";
    }
    // Negative page numbers mean we are counting backwards from the last page...
    else {
      $sql .= "\nORDER BY " . implode(' DESC, ', $order) . " DESC \n";
      $sql .= "OFFSET " . (100 * abs($query_args['page'])) . " LIMIT 100";
    }
  }
  elseif (isset($query_args['page']) AND $query_args['page'] == 'last') {
    $sql .= "\nORDER BY " . implode(' DESC, ', $order) . " DESC \n";
    $sql .= "LIMIT 100";
  }
  else {
    $sql .= "\nORDER BY " . implode(' ASC, ', $order) . " ASC \n";
    $sql .= "LIMIT 100";
  }

  $args = array_merge($call_args, $variant_args);
  // @debug dpm($sql, 'SQL: 1st Query'); dpm($args, 'SQL Arguements: 1st Query');
  $resource = ndg_mview_query($vars['partition'], $sql, $args);

  //dpm(array('sql' => $sql, 'args' => $args),'Variant Query');
  $ids = array(); //Keep track of variant_ids to grab calls for.
  foreach ($resource as $r) {
    $row_id = $r->srcfeature_name . ':' .$r->fmin . '_' . $r->variant_id;
    $vars['variants'][$row_id] = $r;
    $ids[] = $r->variant_id;
  }

  // If this is the last page then we need to reverse the order of the variants.
  if (isset($query_args['page']) AND ($query_args['page'] == 'last' OR $query_args['page'] < 0)) {
    ksort($vars['variants']);
  }

  // Now grab the allele calls.
  if ($ids) {
    if (sizeof($ids) > 1) {
      $sql = "SELECT variant_id, germplasm_id, allele_call \n"
          ."FROM unnest(ARRAY[" . implode(',',$ids) . "]) variant_id \n"
          ."JOIN {mview_ndg_calls} call USING(variant_id) \n"
          ."WHERE " . implode("\n  AND ",$call) . "\n";
    }
    else {
      $sql = "SELECT variant_id, germplasm_id, allele_call \n"
         ."FROM {mview_ndg_calls} call \n"
         ."WHERE call.variant_id=".$ids[0]." AND ".implode("\n AND ",$call) . "\n";
    }
    // @debug dpm(array('sql' => $sql, 'args' => $call_args), 'Call Query');
    $resource = ndg_mview_query($vars['partition'], $sql, $call_args);
    foreach($resource as $r) {
      $vars['data'][$r->variant_id][$r->germplasm_id][] = $r->allele_call;
    }
  }

}

/**
 * Provide an error message for the multiple germplasm form element on the genotype matrix
 * if an entered germplasm does not exist.
 *
 * @param $element
 *   The form element definition.
 * @param $input
 *   The user entered input.
 * @param $index
 *   The index of the multiple stock element which produced this error.
 *
 * @return
 *   A string to be given to form_set_error().
 */
function nd_genotype_matrix_missing_stock_error_callback($element, $input, $index) {
  if ($index == 'new') {

    $suggestions = chado_query('SELECT name FROM {stock} WHERE name~*:input AND type_id NOT IN (SELECT cvterm_id FROM {cvterm} WHERE name=:type) LIMIT 3',
      array(':input' => $input['new']['stock_name'], ':type' => 'DNA'))->fetchCol();

    if ($suggestions) {
      return t('There are no registered genotypes for %name. Did you mean %suggestions?',
        array('%name' => $input['new']['stock_name'], '%suggestions' => implode(', ', $suggestions)));
    }
    else {
      return t('There are no registered genotypes for %name. Perhaps you should check your spelling and ensure you have selected the correct '.$element['#organism_title'].'.',
        array('%name' => $input['new']['stock_name'], '%suggestions' => implode(', ', $suggestions)));
    }
  }
  elseif (isset($input['existing'][$index])) {

    $suggestions = chado_query('SELECT name FROM {stock} WHERE name~*:input AND type_id NOT IN (SELECT cvterm_id FROM {cvterm} WHERE name=:type) LIMIT 3',
      array(':input' => $input['existing'][$index]['stock_name'], ':type' => 'DNA'))->fetchCol();

    if ($suggestions) {
      return t('There are no registered genotypes for %name. Did you mean %suggestions?',
        array('%name' => $input['existing'][$index]['stock_name'], '%suggestions' => implode(', ', $suggestions)));
    }
    else {
      return t('There are no registered genotypes for %name. Perhaps you should check your spelling and ensure you have selected the correct '.$element['#organism_title'].'.',
        array('%name' => $input['existing'][$index]['stock_name'], '%suggestions' => implode(', ', $suggestions)));
    }
  }
  return 'There are no registered genotypes for the germplasm you entered. Perhaps you should check your spelling and ensure you have selected the correct '.$element['#organism_title'].'.';
}
