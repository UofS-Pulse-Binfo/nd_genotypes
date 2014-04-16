<?php

class views_handler_filter_germplasm_genotype_polymorphic extends views_handler_filter_string {

  public $value_form;

  function init(&$view, $options) {
    parent::init($view, $options);

  	// Set admin added germplasm_genotype fields as options
    $this->germplasm_genotype_filters = array();
    foreach ($this->view->display['default']->display_options['filters'] as $name => $filter) {
      if ($filter['field'] == 'germplasm_genotype') {
        $this->germplasm_genotype_filters[$name] = $filter['expose']['identifier'];
      }
    }

  }

  function query () {

    // neither can be <NONE>
    if (!preg_match('/NONE/i',$this->value['first']) OR !preg_match('/NONE/i',$this->value['second'])) {

      // table giving genotype for first stock
      $table_alias = 'nd_genotype_experiment_stock_'.$this->value['first'];

      // add field (all columns in where should also be in fields)
      $this->alias = $this->query->add_field($table_alias,'polymorphic_with');

      // filter join based on stocks selected
			$where = $table_alias .".polymorphic_with ~ :second_stock";
			$this->query->add_where_expression(0, $where, array(':second_stock' => $this->value['second']));

      // ensure there is a genotype for each feature
      // needed so we don't just have blank rows for monomorphic markers
      $where = $table_alias .'.polymorphic_with IS NOT NULL';
      $this->query->add_where_expression(0, $where);

	  }

  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    //unset($form['operator'], $form['case']);
    $this->value_form['#options']['first'] = $this->field_options;
    $this->value_form['#options']['second'] = $this->field_options;

  }


  function value_form (&$form, &$form_state) {
    parent::value_form($form, $form_state);

    // Set options from "Add/Remove Germplasm Genotype Fields"
		//$options = $this->field_options;
		$options = array();
		if (isset($_GET['add_germplasm_genotype']) && is_array($_GET['add_germplasm_genotype'])) {
			foreach ($_GET['add_germplasm_genotype']['existing'] as $g) {
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
