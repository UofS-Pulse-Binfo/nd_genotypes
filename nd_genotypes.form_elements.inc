<?php

/**
 * Register KP Genotype-specific form elements
 */
function nd_genotypes_elements() {
	return array(
	  'organism_stock' => array(
     	'#input' => TRUE,
     	'#process' => array('expand_organism_stock'),
     	'#element_validate' => array('organism_stock_validate'),
  	),
	  'pairwise_compare_germplasm' => array(
     	'#input' => TRUE,
     	'#process' => array('expand_pairwise_compare_germplasm'),
     	'#element_validate' => array('pairwise_compare_germplasm_validate'),
  	),
	  'multiple_organism_stock' => array(
     	'#input' => TRUE,
     	'#process' => array('expand_multiple_organism_stock'),
     	'#element_validate' => array('multiple_organism_stock_validate'),
  	),
  );
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
	if (empty($element['#value'])) {
    	$element['#value'] = array(
      		'organism_id' => '<ANY>',
      		'stock_name' => '',
    	);
  	}
 
	$element['#tree'] = TRUE;
 
	$parents = $element['#parents'];
	$parents[] = 'organism_id';
  $organisms = tripal_core_chado_select('organism',array('genus','species','organism_id'), array());
  $organism_options['<ANY>'] = '<Any>';
  foreach ($organisms as $o) {
    $organism_options[$o->organism_id] = $o->genus .' '. $o->species;
  }
  $element['organism_id'] = array(
    '#type' => 'select',
		'#default_value' => $element['#value']['organism_id'],
    '#prefix' => '<div class="container-inline">Organism:',
    '#suffix' => '</div>',
    '#options' => $organism_options,
  );
 
	$parents = $element['#parents'];
  $parents[] = 'stock_name';
  $element['stock_name'] = array(
    '#type' => 'textfield',
    '#default_value' => $element['#value']['stock_name'],
    '#prefix' => '<div class="container-inline">Name:',
    '#suffix' => '</div>',
    '#size' => 25
  );  
  
  return $element;
}

/**
 * Theme the custom form element
 */
function theme_organism_stock ($element) {
  	return theme('form_element', $element, '<div class="organism-stock-element">'. $element['#children'] .'</div>');
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
	
	if (isset($element['#default_value']) && is_array($element['#default_value'])) {
		// Add name/remove fields for each default value
		foreach ($element['#default_value'] as $k => $g) {
			$element[$k]['prefix'] = array(
				'#type' => 'markup',
				'#value' => '<span class="container-inline" id="multiple-organism-stock-row">'
			);
			
			$element[$k]['name'] = array(
				'#type' => 'item',
				'#value' => $g['stock_name'],
			);
	
			$element[$k]['stock_name'] = array(
				'#type' => 'hidden',
				'#value' => $g['stock_name'],
			);
	
			$element[$k]['stock_id'] = array(
				'#type' => 'hidden',
				'#value' => $g['stock_id'],
			);
			
			$element[$k]['organism_id'] = array(
				'#type' => 'hidden',
				'#value' => $g['organism_id'],
			);
			
			$element[$k]['remove'] = array(
				'#type' => 'checkbox',
				'#title' => 'Remove',
			);
	
			$element[$k]['suffix'] = array(
				'#type' => 'markup',
				'#value' => '</span><br />'
			);
		}
	}

	$element['add_stock'] = array(
		'#type' => 'organism_stock',
		'#title' => '<span id="multiple-add-organism-stock">Add</span>'
	);

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
function multiple_organism_stock_validate ($element) {

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
  $element['#options']['first']['<NONE>'] = '<None>';
  $element['#options']['second']['<NONE>'] = '<None>';
  
	$element['#tree'] = TRUE;
  
	$parents = $element['#parents'];
	$parents[] = 'first';
  $element['first'] = array(
    '#type' => 'select',
		'#default_value' => $element['#value']['first'],
    '#options' => $element['#options']['first'],
  );
 
	$parents[] = 'second';
  $element['second'] = array(
    '#type' => 'select',
		'#default_value' => $element['#value']['second'],
    '#options' => $element['#options']['second'],
    '#prefix' => ' vs. '
  ); 
  
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
  
  // Remove generic "An illegal choice has been detected. Please contact the site administrator." message.
  // This is shown because the options array is not of the expected format
  if (is_array($_SESSION['messages']['error'])) {
    $key = array_search('An illegal choice has been detected. Please contact the site administrator.',$_SESSION['messages']['error']);
    unset($_SESSION['messages']['error'][$key]);
    if (empty($_SESSION['messages']['error'])) {
      unset($_SESSION['messages']['error']);
    }
  }
    
  // Furthermore, remove highlighting of error form elements
  // -current form errors
  $form = form_set_error();
  // - remove one for this element
  unset($form[ implode('][',$element['#parents']) ]);
  // -reset form error cache
  form_set_error(NULL,'',TRUE);
  // -add all errors (except one for this element) back
  foreach ($form as $name => $message) {
    form_set_error($name, $message);
  }

  
  //Determine whether the submitted value is in the options array
  // If not emit our own "An illegal choice has been detected..." message
  if (!isset($element['#options']['first'][ $element['#value']['first'] ])) {
    form_set_error( $element['#parents'][0].'][first', 'An illegal choice has been detected. Please contact the site administrator.');
  }
  if (!isset($element['#options']['second'][ $element['#value']['second'] ])) {
    form_set_error( $element['#parents'][0].'][second', 'An illegal choice has been detected. Please contact the site administrator.');
  }  
 
}
