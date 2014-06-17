<?php

function nd_genotypes_test_form($form, $form_state) {

  $form['t'] = array(
    '#type' => 'textfield',
    '#title' => 'Name',
  );
/**
  $form['stock'] = array(
    '#type' => 'organism_stock',
    '#title' => t('Individual'),
    '#default_value' => array(
      'stock_name' => 'test stock name',
      'organism_id' => 4
    ),
  );
*/
  $form['multiple_stocks'] = array(
			'#type' => 'multiple_organism_stock',
			'#default_value' => (isset($form_state['values']['element'])) ? $form_state['values']['element'] : '',
	);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit'
  );

  return $form;
}

function nd_genotypes_test_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Register KP Genotype-specific form elements
 */
function nd_genotypes_element_info() {
  $elements = array();

  $elements['organism_stock'] = array(
     	'#input' => TRUE,
     	'#process' => array('expand_organism_stock'),
     	'#element_validate' => array('organism_stock_validate'),
     	'#theme' => array('organism_stock'),
      '#theme_wrappers' => array('form_element'),
      '#tree' => TRUE,
  );

  $elements['pairwise_compare_germplasm'] = array(
     	'#input' => TRUE,
     	'#process' => array('expand_pairwise_compare_germplasm'),
     	'#element_validate' => array('pairwise_compare_germplasm_validate'),
     	'#theme' => array('pairwise_compare_germplasm'),
      '#theme_wrappers' => array('form_element'),
      '#tree' => TRUE,
  );

  $elements['multiple_organism_stock'] = array(
     	'#input' => TRUE,
     	'#process' => array('expand_multiple_organism_stock'),
     	'#element_validate' => array('multiple_organism_stock_validate'),
     	'#value_callback' => 'multiple_organism_stock_value_callback',
     	'#theme' => array('multiple_organism_stock'),
      '#theme_wrappers' => array('form_element'),
      '#tree' => TRUE,
  );

  return $elements;
}

function nd_genotypes_form_element_theme () {
  return array(
    'organism_stock' => array(
      'arguments' => array('element' => NULL)
    ),
    'pairwise_compare_germplasm' => array(
      'arguments' => array('element' => NULL)
    ),
    'multiple_organism_stock' => array(
      'arguments' => array('element' => NULL)
    ),
  );
}

///////////////////////////////////////////////////////////
// Form element: organism_stock
///////////////////////////////////////////////////////////

/**
 * Custom form element with an organism select box and a stock name textfield
 *
 * Example on how to use it:
 * @code
   $form['stock'] = array(
    '#type' => 'organism_stock',
    '#title' => t('Individual'),
    '#default_value' => array(
      'stock_name' => $form_state['values']['stock']['stock_name'],
      'organism_id' => $form_state['values']['stock']['organism_id']
    ),
  );
 * @endcode
 */
function expand_organism_stock ($element) {

	// Assigning default values to element. You can override these values by passing your own values using "#default_value".
	if (empty($element['#default_value'])) {
    $element['#value'] = array(
        'organism_id' => 0,
        'stock_name' => '',
    );
  }
  else {
    $element['#value'] = $element['#default_value'];
  }

  if (!isset($element['#required'])) {
    $element['#required'] = FALSE;
  }

	$element['#tree'] = TRUE;

	$parents = $element['#parents'];
	$parents[] = 'organism_id';
	if (isset($element['#organism_options'])) {
	  $organism_options = $element['#organism_options'];
	}
	else {
    $organisms = chado_select_record('organism',array('genus','species','organism_id'), array(), array('order_by'=>array('genus' => 'ASC', 'species' => 'ASC')));
    $organism_options[0] = ' ';
    foreach ($organisms as $o) {
      $organism_options[$o->organism_id] = $o->genus .' '. $o->species;
    }
  }
  $element['organism_id'] = array(
    '#title' => (isset($element['#organism_title'])) ? $element['#organism_title'] : 'Organism',
    '#type' => 'select',
		'#default_value' => $element['#value']['organism_id'],
    '#prefix' => '<div class="organism-stock-organism_id organism-stock-element">',
    '#suffix' => '</div>',
    '#options' => $organism_options,
    '#required' => $element['#required'],
    '#weight' => (isset($element['#organism_weight'])) ? $element['#organism_weight'] : 0,
  );

	$parents = $element['#parents'];
  $parents[] = 'stock_name';
  $element['stock_name'] = array(
    '#title' => 'Stock Name',
    '#type' => 'textfield',
    '#default_value' => $element['#value']['stock_name'],
    '#prefix' => '<div class="organism-stock-name organism-stock-element">',
    '#suffix' => '</div>',
    '#size' => 25,
    '#required' => $element['#required'],
    '#weight' => 0
  );

  return $element;
}

/**
 * Theme the custom form element
 */
function theme_organism_stock ($element) {
  	return theme('form_element', $element, '<div class="organism-stock">'. $element['#children'] .'</div>');
}

/**
 * Validate user-input for all organism_stock form elements
 */
function organism_stock_validate ($element) {

}

///////////////////////////////////////////////////////////
// Form element: multiple_organism_stock
///////////////////////////////////////////////////////////

/**
 * Custom form element that keeps track of multiple organism_stocks (see above form element).
 * There is a list of already added stock names with the ability to remove them by selecting a
 * checkbox and an organism_stock form element to add to the list.
 *
 * Example of how to use it:
 * @code
		$form['element'] = array(
			'#type' => 'multiple_organism_stock',
			'#default_value' => $form_state['values']['element'],
		);
 * @endcode
 */
function expand_multiple_organism_stock ($element) {

	$parents = $element['#parents'];
	$element['#tree'] = TRUE;

  // if there is a value then add it to the default value
  if (isset($element['#value']['stock_id'])) {
    $element['$default_value'] = $element['#value'];
  }

	if (isset($element['#default_value']) && is_array($element['#default_value'])) {

		// Add name/remove fields for each default value
		if (!isset($element['#default_value']['existing'])) {
		  $element['#default_value']['existing'] = array();
		}
		foreach ($element['#default_value']['existing'] as $k => $g) {

      if (!isset($g['stock_id'])) {
        continue;
      }

		  $element['existing']['#type'] = 'fieldset';
      $element['existing']['#title'] = 'Previously Added Stocks';
      $element['existing']['#description'] = 'The following stocks have already been added:';
      if (isset($element['#added_description'])) {
        $element['existing']['#description'] = $element['#added_description'];
      }

			$element['existing'][$k] = array(
			  'prefix' => array(
				  '#type' => 'markup',
				  '#markup' => '<span class="container-inline" id="multiple-organism-stock-row">'
			  ),
        'name' => array(
				  '#type' => 'item',
				  '#markup' => $g['stock_name'],
			  ),
			  'stock_name' => array(
				  '#type' => 'hidden',
				  '#value' => $g['stock_name'],
			  ),
			  'stock_id' => array(
				  '#type' => 'hidden',
				  '#value' => $g['stock_id'],
			  ),
			  'organism_id' => array(
				  '#type' => 'hidden',
				  '#value' => $g['organism_id'],
			  ),
			  'remove' => array(
				  '#type' => 'checkbox',
				  '#title' => 'Remove',
			  ),
			  'suffix' => array(
				  '#type' => 'markup',
				  '#markup' => '</span><br />'
				),
			);
		}
	}

	$element['add_stock'] = array(
		'#type' => 'organism_stock',
	);

	if (isset($element['#organism_title'])) {
	  $element['add_stock']['#organism_title'] = $element['#organism_title'];
	}
  if (isset($element['#organism_weight'])) {
	  $element['add_stock']['#organism_weight'] = $element['#organism_weight'];
	}
	if (isset($element['#organism_options'])) {
	  $element['add_stock']['#organism_options'] = $element['#organism_options'];
	}

  return $element;
}

/**
 * Theme the custom form element
 */
function theme_multiple_organism_stock ($element) {
  	return theme('form_element', $element, '<div class="multiple-organism-stock-element">'. $element['#children'] .'</div>');
}

/**
 * Validate user-input for all multiple_organism_stock form elements
 */
function multiple_organism_stock_validate (&$element) {

  $input = $element['#value'];

  if (!isset($input['add_stock'])) {
    $input['add_stock'] = array(
      'stock_name' => '',
      'organism_id' => 0
    );
  }

  // If both the name/organism of a stock has been added
  // then add it to 'existing' stocks
  if (($input['add_stock']['organism_id'] != 0) AND !empty($input['add_stock']['stock_name'])) {

    // First we need to get the stock referenced
    $stock = tripal_get_stock(array(
      'name' => $input['add_stock']['stock_name'],
      'organism_id' => $input['add_stock']['organism_id']
    ));
    if (!$stock) {
      form_set_error("add_stock']['stock_name", 'There is no stock with the entered name and organism combination');
    }
    else {
      // This is not getting saved :(
      $element['#value']['add_stock']['stock_id'] = $stock->stock_id;
    }
  }

  return $element;
}

function multiple_organism_stock_value_callback($element, $input = FALSE, &$form_state) {

}

///////////////////////////////////////////////////////////
// Form element: pairwise_compare_germplasm
///////////////////////////////////////////////////////////

/**
 * Custom form element allowing you to select two stocks to be compared from two select lists.
 *
 * Example on how to use it:
 * @code
    $options = array(
      'stock1',stock2','stock3'
    );
		$form['element'] = array(
			'#type' => 'pairwise_compare_germplasm',
			'#options' => array(
			  'first' => $options,
			  'second' => $options
			),
			'#default_value' => $form_state['values']['element'],
		);
 * @endcode
 * Will give you two select lists where each select list contains the items in $options.
 */
function expand_pairwise_compare_germplasm ($element) {

	// Assigning default values to element. You can override these values by passing your own values using "#default_value".
	if (empty($element['#value'])) {
    	$element['#value'] = array(
      		'first' => '<NONE>',
      		'second' => '<NONE>',
    	);
  	}

  // Add <none> to options
  $element['#options']['first']['<NONE>'] = '';
  $element['#options']['second']['<NONE>'] = '';

	$element['#tree'] = TRUE;

	$parents = $element['#parents'];
	$parents[] = 'first';
  $element['first'] = array(
    '#type' => 'select',
		'#default_value' => $element['#value']['first'],
    '#options' => $element['#options']['first'],
    '#prefix' => 'Between ',
    '#validated' => TRUE
  );

	$parents[] = 'second';
  $element['second'] = array(
    '#type' => 'select',
		'#default_value' => $element['#value']['second'],
    '#options' => $element['#options']['second'],
    '#prefix' => ' and ',
    '#validated' => TRUE
  );

  $element['#validated'] = TRUE;

  return $element;
}

/**
 * Theme the custom form element
 */
function theme_pairwise_compare_germplasm ($element) {
  	return theme('form_element', $element, '<div class="pairwise_compare_germplasm-element container-inline">'. $element['#children'] .'</div>');
}

/**
 * Validate user-input for all pairwise_compare_germplasm form elements
 */
function pairwise_compare_germplasm_validate ($element) {

}
