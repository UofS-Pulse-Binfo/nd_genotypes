<?php

class views_handler_filter_germplasm_genotype_polymorphic extends views_handler_filter_string {

  public $value_form;
    
  function init(&$view, $options) {
    parent::init($view, $options);
  
  	// Set admin added germplasm_genotype fields as options
    $this->germplasm_genotype_filters = array();
    //dpm($this->view, 'view');
    foreach ($this->view->display['default']->display_options['filters'] as $name => $filter) {
      //dpm($filter, 'filter');
      if ($filter['field'] == 'germplasm_genotype') {
        $this->germplasm_genotype_filters[$name] = $filter['expose']['identifier'];
      }
    }

  }
  
  function query () {

    // neither can be <NONE>
    if (!preg_match('/NONE/i',$this->value['first']) OR !preg_match('/NONE/i',$this->value['second'])) {
      
      dpm($this->value, 'values in query');
					
      // Make our own join
      // (don't just ensure_my_table b/c this way we can limit to only showing genotype_description)
      $table_alias = 'nd_genotype_polymorphism_'.$this->value['first'].'_by_'.$this->value['second'];
      $def = array();
      $def['table'] = 'nd_genotype_polymorphism';
      $def['field'] = 'feature_id';
      $def['left_table'] = 'feature';
      $def['left_field'] = 'feature_id';
  
      $join = new views_join();
      $join->definition = $def;
      $join->construct();
      $join->adjusted = TRUE;
  
      $this->query->add_relationship($table_alias, $join, $def['table']);
      
      // add field (all columns in where should also be in fields)
      $this->alias = $this->query->add_field($table_alias,'is_polymorphic');
  
      // filter join based on stocks selected
      if ($this->value['first'] < $this->value['second']) {
        $where = $table_alias .'.stockA_id = '.$this->value['first'];
        $this->query->add_where($group, $where);
        $where = $table_alias .'.stockB_id = '.$this->value['second'];
        $this->query->add_where($group, $where);
      } else {
        $where = $table_alias .'.stockA_id = '.$this->value['second'];
        $this->query->add_where($group, $where);
        $where = $table_alias .'.stockB_id = '.$this->value['first'];
        $this->query->add_where($group, $where);
      }
            
      // add where is_polymorphic = TRUE
      // since field is a boolean we don't need to make a comparison
      $where = $table_alias .'.is_polymorphic';
      $this->query->add_where($group, $where);

	  }
	  
  }
  
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    unset($form['operator'], $form['case']);
    $this->value_form['#options']['first'] = $this->field_options;
    $this->value_form['#options']['second'] = $this->field_options;
    
  }

  
  function value_form (&$form, &$form_state) {
    parent::value_form($form, $form_state);

    // Set options from "Add/Remove Germplasm Genotype Fields"
		$options = $this->field_options;
		if (isset($_GET['add_germplasm_genotype']) && is_array($_GET['add_germplasm_genotype'])) {
			foreach ($_GET['add_germplasm_genotype'] as $g) {
				$options[ $g['stock_id'] ] = $g['stock_name'];
			}
		}

    // Set Options from Germplasm Genotype Filters
    if (isset($this->germplasm_genotype_filters)) {
      foreach ($this->germplasm_genotype_filters as $filter => $identifier) {
        if (isset($_GET[$identifier]['stock_name']) AND isset($_GET[$identifier]['organism_id'])) {
          $stock = tripal_stock_get_stock_by_name_identifier($_GET[$identifier]['stock_name'], $_GET[$identifier]['organism_id']);
          if ($stock[0]->stock->stock_id) {
            $options[ $stock[0]->stock->stock_id ] = $_GET[$identifier]['stock_name'];
          }
        }
      }
    }
    		
    $this->value_form = array(
      '#type' => 'pairwise_compare_germplasm',
			'#title' => 'Value',
			'#default_value' => $this->value,
			'#options' => array(
				'first' => $options,
				'second' => $options
			),
    );    
    $form['value'] = &$this->value_form;
    
  }

}
