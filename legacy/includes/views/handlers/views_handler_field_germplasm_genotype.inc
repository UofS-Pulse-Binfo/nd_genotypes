<?php

/**
 * Defines a field which displays the genotypes of a specified germplasm for the features listed
 * in the view
 *
 * NOTE: This field does not add to the main views query but instead executes it's own query
 * on a materialized view (kp_stock_genotype_counts)
 */
class views_handler_field_germplasm_genotype extends views_handler_field_prerender_list {

  /**
   * {@inheritdoc}
   */
  function init(&$view, &$options) {
    parent::init($view, $options);

    // Expose the field if needed.
    if (isset($this->original_stock_id)) {
      $this->options['exclude'] == 0;
    }
  }

  /**
   * {@inheritdoc}
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['settings']['germplasm'] = array(
      'export' => TRUE
    );
    $options['settings']['use_filter'] = array(
      'default' => '<NONE>',
      'export' => TRUE
    );
    $options['settings']['has_genotype'] = array(
      'default' => 1,
      'export' => TRUE
    );
    $options['settings']['get_selections'] = array(
      'default' => 1,
      'export' => TRUE
    );

    $options['render']['render_type'] = array(
      'default' => 'both',
      'export' => TRUE
    );
    $options['render']['color_bases'] = array(
      'default' => TRUE,
      'bool' => TRUE,
      'export' => TRUE
    );

    return $options;
  }

  /**
   * {@inheritdoc}
   *
   * Adds a number of fields to the option form (displayed to admin to configure field)
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => 'Genotype Source'
    );

    // specify the germplasm to show genotypes for
    $form['settings']['germplasm'] = array(
      '#type' => 'organism_stock',
      '#title' => 'Default Germplasm',
      '#description' => 'Enter the default germplasm genotypes should be shown for.',
      '#default_value' => $this->options['settings']['germplasm'],
    );

    // set the germplasm_genotype filter that can change the above default
    $options['<NONE>'] = 'None';
    if (isset($this->view->display['default']->display_options['filters'])) {
      foreach ($this->view->display['default']->display_options['filters'] as $name => $filter) {
        if ($filter['field'] == 'germplasm_genotype' OR $filter['field'] == 'add_germplasm_genotype') {
          if (isset($filter['expose'])) {
            $options[$name] = $filter['expose']['label'];
          } else {
            $options[$name] = $name;
          }
        }
      }
    }
    $form['settings']['use_filter'] = array(
      '#type' => 'select',
      '#title' => t('Use Germplasm Genotype Filter'),
      '#description' => 'Select a filter which can be used to change the default germplasm set above.',
      '#options' => $options,
      '#default_value' => $this->options['settings']['use_filter'],
    );

    // ensure that only features with genotypes associated with the current germplasm are listed
    $form['settings']['has_genotype'] = array(
      '#type' => 'checkbox',
      '#title' => 'Require genotype exists for this stock',
      '#default_value' => (isset($this->options['settings']['has_genotype'])) ? isset($this->options['settings']['has_genotype']) : 1,
    );

    // pulls in genotypes from selections of the current stock
    // ie: from stocks connected to the current stock (object) via is_selection_of (type) relationship
    $form['settings']['get_selections'] = array(
      '#type' => 'checkbox',
      '#title' => 'Display genotypes from selections of the current stock',
      '#description' => 'Adds genotypes from any stock that is the subject in a <subject> is_selection_of <current stock> relationship.',
      '#default_value' => (isset($this->options['settings']['get_selections'])) ? isset($this->options['settings']['get_selections']) : 1,
    );

    $form['render'] = array(
      '#type' => 'fieldset',
      '#title' => 'Render Settings'
    );

    // set the way the genotype is rendered
    // useful when there is 2+ genotypes per marker per germplasm combination
    $form['render']['render_type'] = array(
      '#type' => 'radios',
      '#title' => 'Render Multiple Genotypes as',
      '#options' => array(
        'list' => 'List multiple Genotypes separated as indicated by display type (ie: A, T, A)',
        'consensus' => 'Display the Consensus Genotype (ie: A)',
        'ratio' => 'Display the ratio of genotypes (ie: 66% A : 33% T)',
        'both' => 'Display the consensus followed by the ratio of genotypes (ie: A (66% A : 33% T))'
      ),
      '#default_value' => ($this->options['render']['render_type']) ? $this->options['render']['render_type'] : 'both'
    );

    // If checked will colour the bases according to the sanger sequencing standards
    // (ie: C: blue, G: green, A: black, T: red)
    // Uses a CSS rule
    $form['render']['color_bases'] = array(
      '#type' => 'checkbox',
      '#title' => 'Color bases',
      '#default_value' => ($this->options['render']['color_bases']) ? $this->options['render']['color_bases'] : TRUE
    );

  }

  /**
   * {@inheritdoc}
   *
   * Actually save the options set in option_form()
   */
  function options_submit(&$form, &$form_state) {

    $this->options['settings']['germplasm'] = $form_state['values']['options']['settings']['germplasm'];
    $this->options['settings']['use_filter'] = $form_state['values']['options']['settings']['use_filter'];
    $this->options['settings']['has_genotype'] = $form_state['values']['options']['settings']['has_genotype'];
    $this->options['settings']['get_selections'] = $form_state['values']['options']['settings']['get_selections'];

    $this->options['render']['render_type'] = $form_state['values']['options']['render']['render_type'];
    $this->options['render']['color_bases'] = $form_state['values']['options']['render']['color_bases'];

  }

  /**
   * {@inheritdoc}
   *
   * Set the label and ensure the genotypes exist (if that option is checked)
   */
  function query() {
    $this->aliases['feature_id'] = 'feature_id';
    $this->field_alias = $this->aliases['feature_id'];

    // Set aliases
    $this->aliases['genotypes'] = $this->options['id'];
    $this->aliases['feature_id'] = 'feature_id';
    $this->field_alias = $this->aliases['feature_id'];

    // Determine stock name and organism_id and stock_id
    $this->set_stock_details();

    // Then if we don't have a stock, don't add anything to the query.
    if (!isset($this->original_stock_id)) {
      return FALSE;
    }

    //Set label to be stock_name
    if (!empty($this->original_stock_name)) {
      foreach ($this->view->display as $display) {
        $display->handler->options['fields'][ $this->aliases['genotypes'] ]['label'] = $this->original_stock_name;
      }
      $this->options['label'] = $this->original_stock_name;
    }

    // Add additional stocks which are selections of the current stock
    $selections = chado_select_record(
      'stock_relationship',
      array('subject_id'),
      array('object_id' => $this->original_stock_id, 'type_id' => array('name' => 'is_selection_of'))
    );
    foreach ($selections as $s) {
      $stock = chado_select_record('stock',array('stock_id','organism_id','name'),array('stock_id'=>$s->subject_id));
      if ($stock[0]->organism_id) {
        $this->stock_id[ $stock[0]->stock_id ] = $stock[0]->stock_id;
        $this->stock_name[ $stock[0]->stock_id ] = $stock[0]->name;
        $this->organism_id[ $stock[0]->stock_id ] = $stock[0]->organism_id;
      }
    }

    // Add to query
    $this->added_to_query = FALSE;
    if (sizeof($this->stock_id) > 0) {

      // Make our own join
      // (don't just ensure_my_table b/c this way we can limit to only showing genotype_description)
      $table_alias = 'nd_genotype_experiment_stock_'.$this->original_stock_id;
      $def = array();
      $def['table'] = 'nd_genotype_experiment';
      $def['field'] = 'feature_id';
      $def['left_table'] = 'feature';
      $def['left_field'] = 'feature_id';

      $join = new views_join();
      $join->definition = $def;
      $join->construct();
      $join->adjusted = TRUE;

      $this->query->add_relationship($table_alias, $join, $def['table']);

      // Now we need to remove the original join added via the relationship
      // otherwise we end up with repetitive rows
      if (isset($this->query->table_queue['nd_genotype_experiment_feature'])) {
        // but only remove this if there are no filters/fields using it
        // thus we need to check all conditions on this query for the nd_genotype_experiment_feature prefix
        foreach ($this->query->where as $group_id => $group_details) {
          foreach($group_details['conditions'] as $index => $condition) {
            if (preg_match('/^nd_genotype_experiment_feature\.(\w+)/', $condition['field'], $matches)) {
              $this->query->where[$group_id]['conditions'][$index]['field'] = $table_alias . '.' . $matches[1];
            }
          }
        }
        unset(
          $this->query->table_queue['nd_genotype_experiment_feature'],
          $this->query->tables['feature']['nd_genotype_experiment_feature'],
          $this->query->relationships['nd_genotype_experiment_feature']
        );
      }

      // The above join needs to be restricted to the stock set
      $this->options['group'] = (isset($this->options['group'])) ? $this->options['group'] : 0;
      $field = $table_alias . '.stock_id';
      $operator = (sizeof($this->stock_id) == 1) ? '=' : 'IN';

      // genotype_description field
      //$field = $table_alias . '.genotype_description';
      $this->genotype_allele_field_alias = $this->query->add_field(
        $table_alias,
        'genotype_description',
        'stock'.$this->original_stock_id.'_allele',
        array('function' => 'array_agg')
      );
      $this->field_alias = $this->genotype_allele_field_alias;

      // genotype_id field
      $field = $table_alias . '.genotype_id';
      $this->genotype_id_field_alias = $this->query->add_field($table_alias, 'genotype_id', 'stock'.$this->original_stock_id.'_genotype_id', array('function' => 'array_agg'));

      // stock_id field
      $field = $table_alias . '.stock_id';
      $this->stock_id_field_alias = $this->query->add_field($table_alias, 'stock_id', 'stock'.$this->original_stock_id.'_stock_id', array('function' => 'array_agg'));
      $this->query->add_where($this->options['group'], $field, $this->original_stock_id, $operator);

      $this->added_to_query = TRUE;

    }

    // Change the alias of the feature.name field if it's added to the current view
    // in order to avoid ambiguous feature_name errors in the GroupBy
    // (there's also a nd_genotype_experiment.feature_name)
    $this->view->query->fields['feature_name']['alias'] = 'feature_feature_name';
    $this->view->field['name']->field_alias = 'feature_feature_name';
    foreach ($this->view->display as $display) {
      $display->handler->options['fields']['name']['id'] = 'feature_feature_name';
    }
  }

  /**
   * {@inheritdoc}
   *
   * Add genotypes to $view->result (results of the query) and $this->items (used in rendering)
   */
  function pre_render(&$values) {

    // Expose the field if needed.
    if (isset($this->original_stock_id)) {
      $this->options['exclude'] = 0;
    }

    //dpq($this->view->build_info['query']);

    if (!isset($this->added_to_query)) $this->added_to_query = FALSE;

    if ($this->added_to_query) {
      foreach ($values as $k => $v ) {

        // Translate to PHP array
        $values[$k]->{$this->genotype_allele_field_alias} = views_list_aggregation_aggregation_postgres2php_array($v->{$this->genotype_allele_field_alias});
        $values[$k]->{$this->genotype_id_field_alias} = views_list_aggregation_aggregation_postgres2php_array($v->{$this->genotype_id_field_alias});

        // If there is ONLY 1 genotype for this marker
        if (is_string($values[$k]->{$this->genotype_allele_field_alias})) {
          $genotype_id = $values[$k]->{$this->genotype_id_field_alias};
          $allele = $values[$k]->{$this->genotype_allele_field_alias};
          $this->items[$v->feature_feature_id][$genotype_id]['feature_id'] = $v->feature_feature_id;
          $this->items[$v->feature_feature_id][$genotype_id]['genotype_id'] = $genotype_id;
          $this->items[$v->feature_feature_id][$genotype_id]['genotype'] = $allele;
          $this->items[$v->feature_feature_id][$genotype_id]['percent_per_genotype_per_stock'] = 100;

          if (isset($this->counts[ $v->feature_feature_id ][ $genotype_id ])) {
            $this->counts[ $v->feature_feature_id ][ $genotype_id ]++;
          }
          else {
            $this->counts[ $v->feature_feature_id ][ $genotype_id ] = 1;
          }

        }
        // If there is 2+ genotype for this marker
        elseif (is_array($values[$k]->{$this->genotype_allele_field_alias})) {

          $num_genotypes = sizeof(array_unique($values[$k]->{$this->genotype_allele_field_alias}));

          foreach ($values[$k]->{$this->genotype_allele_field_alias} as $sk => $allele) {
            $genotype_id = $values[$k]->{$this->genotype_id_field_alias}[$sk];
            $allele = $values[$k]->{$this->genotype_allele_field_alias}[$sk];
            $this->items[$v->feature_feature_id][$genotype_id]['feature_id'] = $v->feature_feature_id;
            $this->items[$v->feature_feature_id][$genotype_id]['genotype_id'] = $genotype_id;
            $this->items[$v->feature_feature_id][$genotype_id]['genotype'] = $allele;
            // This metric doesn't take into account that the same allele
            // might be called multiple times...
            $this->items[$v->feature_feature_id][$genotype_id]['percent_per_genotype_per_stock'] = 1 / $num_genotypes * 100;


            if (isset($this->counts[ $v->feature_feature_id ][ $genotype_id ])) {
              $this->counts[ $v->feature_feature_id ][ $genotype_id ]++;
            }
            else {
              $this->counts[ $v->feature_feature_id ][ $genotype_id ] = 1;
            }
          }
        }
      }
		}
  }

  /**
   * Return an array of items for the field.
   *
   * Items should be stored in the result array, if possible, as an array
   * with 'value' as the actual displayable value of the item, plus
   * any items that might be found in the 'alter' options array for
   * creating links, such as 'path', 'fragment', 'query' etc, such a thing
   * is to be made. Additionally, items that might be turned into tokens
   * should also be in this array.
   */
  function get_items($values) {
    if (isset($this->items[$values->feature_feature_id])) {
      return $this->items[$values->feature_feature_id];
    }
    else {
      return array();
    }
  }

  /**
   * Render all items in this field together.
   *
   * When using advanced render, each possible item in the list is rendered
   * individually. Then the items are all pasted together.
   */
  function render_items($items) {

    if (empty($items)) {
      return '';
    }

    if ((sizeof($items) == 1) OR ($this->options['render']['render_type'] == 'list')) {
      $rendered_items = parent::render_items($items);

      // colour all bases
      if ($this->options['render']['color_bases']) {
        $rendered_items = str_replace('T', "<span id='genotype' class='T'>T</span>", $rendered_items);
        $rendered_items = str_replace('A', "<span id='genotype' class='A'>A</span>", $rendered_items);
        $rendered_items = str_replace('G', "<span id='genotype' class='G'>G</span>", $rendered_items);
        $rendered_items = str_replace('C', "<span id='genotype' class='C'>C</span>", $rendered_items);
      }

    } else {

      $max_percent = 0;
      $consensus = '';
      $parts = array();
      foreach ($items as $i) {
        preg_match('/^(\d+\.*\d*)% (.*)$/',$i, $matches);
        if ($max_percent < $matches[1]) {
          $max_percent = $matches[1];
          if (!preg_match('/\?/',$matches[2])) {
            $consensus = $matches[2];
          }
        }
      }
      // just in case all of them are questionable
      // assign the last one so it's not blank
      if (!$consensus) {
        $consensus = $matches[2];
      }

      if ($this->options['render']['color_bases']) {
        $consensus = str_replace('T', "<span id='genotype' class='T'>T</span>", $consensus);
        $consensus = str_replace('A', "<span id='genotype' class='A'>A</span>", $consensus);
        $consensus = str_replace('G', "<span id='genotype' class='G'>G</span>", $consensus);
        $consensus = str_replace('C', "<span id='genotype' class='C'>C</span>", $consensus);
      }

      switch($this->options['render']['render_type']) {
        case 'consensus':
          $rendered_items = $consensus;
          break;
        case 'ratio':
          $rendered_items = implode(' : ',$items);
          break;
        case 'both':
          $rendered_items = $consensus . ' (' . implode(' : ',$items) . ')';
          break;
        default:
          $rendered_items = $consensus . ' (' . implode(' : ',$items) . ')';
          break;
      }

    }

    return $rendered_items;
  }

  /**
   * {@inheritdoc}
   *
   * Render a single item
   */
  function render_item($count, &$item) {
    $output = '';

    // Set the percent of the item
    // needed to be done here so we had the full count of stocks with genotypes
    if (sizeof($this->stock_id) == 1) {
      $item['percent'] = $item['percent_per_genotype_per_stock'];
    } else {
      $percent = ($item['percent_per_genotype_per_stock'] / sizeof($this->counts[ $item['feature_id'] ]) ) * 100;
      $item['percent'] = $percent;
    }

    // render the items
    if ($item['percent'] == '100') {
      $output = $item['genotype'];
    } else {
      $output = $item['percent'] . '% ' . $item['genotype'];
    }

    return $output;
  }

  /**
   * Sets the stock details based on the exposed filters or defaults.
   */
  function set_stock_details() {

    if ($this->options['settings']['use_filter'] != '<NONE>') {
      $filter = $this->view->display['default']->display_options['filters'][ $this->options['settings']['use_filter'] ];
      $filter_identifier = $filter['expose']['identifier'];

      // If it has a value in the path.
      if (isset($_GET[ $filter_identifier ])) {
        // Extract the stock information based on the filter type.
        if ($filter['field'] == 'germplasm_genotype') {
          $filter_value = $_GET[ $filter_identifier ];
          $this->stock_name = $filter_value['stock_name'];
          $this->organism_id = $filter_value['organism_id'];
        }
        elseif ($filter['field'] == 'add_germplasm_genotype') {
          $filter_value = $_GET[ $filter_identifier ];

          // Determine which germplasm genotype field this is.
          if(preg_match('/germplasm_genotype_(\d+)/', $this->options['id'], $match)) {
            $index = $match[1];
          }
          elseif ($this->options['id'] == 'germplasm_genotype') {
            $index = 0;
          }

          if (isset($filter_value['existing'][$index])) {
            $this->stock_name = $filter_value['existing'][$index]['stock_name'];
            $this->organism_id = $filter_value['existing'][$index]['organism_id'];
            $this->stock_id = $filter_value['existing'][$index]['stock_id'];
          }
        }
      }
      elseif (!empty($this->options['settings']['germplasm']['stock_name'])) {
        $this->stock_name = $this->options['settings']['germplasm']['stock_name'];
        $this->organism_id = $this->options['settings']['germplasm']['organism_id'];
      }
    }
    elseif (!empty($this->options['settings']['germplasm']['stock_name'])) {
      $this->stock_name = $this->options['settings']['germplasm']['stock_name'];
      $this->organism_id = $this->options['settings']['germplasm']['organism_id'];
    }

    if (isset($this->stock_name)) {
      if (isset($this->stock_id)) {
        $stock_id = $this->stock_id;
        $this->original_stock_id = $stock_id;
        $this->original_stock_name = $this->stock_name;
        $this->stock_id = array($stock_id => $stock_id);
        $this->stock_name = array($stock_id => $this->stock_name);
        $this->organism_id = array($stock_id => $this->organism_id);
      }
      else {
        $stock = tripal_get_stock(array('name' => $this->stock_name, 'organism_id' => $this->organism_id));
        if ($stock) {
          if (is_object($stock)) {
            $this->stock_id[$stock->stock_id] = $stock->stock_id;
            $this->stock_name[$stock->stock_id] = $stock->name;
            $this->organism_id[$stock->stock_id] = $stock->organism_id;
            $this->original_stock_id = $stock->stock_id;
            $this->original_stock_name = $stock->name;
          }
          else {

          }
        }
        else {
          drupal_set_message('There are no stocks with the name, "'.$this->stock_name.'", associated with that organism.', 'error');
        }
      }
    }
  }

}
