<?php
/**
 * @file
 * Default Views related to ND Genotypes
 */

/**
 * Implements hook_views_default_views().
 */
function nd_genotypes_views_default_views() {

  $view = nd_genotypes_genotype_matrix_defaultview();
  $views[$view->name] = $view;

  $view = nd_genotypes_genotype_matrix_germplasm_list_defaultview();
  $views[$view->name] = $view;

  $view = nd_genotypes_feature_genotype_chart_pie_defaultview();
  $views[$view->name] = $view;

  $view = nd_genotypes_genotype_admin_defaultview();
  $views[$view->name] = $view;

  return $views;
}

/**
 * Genotype Distribution on Feature Marker/Variant pages.
 */
function nd_genotypes_feature_genotype_chart_pie_defaultview() {

  $view = new view();
  $view->name = 'nd_genotypes_feature_genotype_chart_pie';
  $view->description = 'Pie chart of genotypes on feature pages';
  $view->tag = 'ND Genotypes';
  $view->base_table = 'nd_genotype_experiment';
  $view->human_name = 'Feature: Genotype Distribution';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['group_by'] = TRUE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'none';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['style_plugin'] = 'chart';
  $handler->display->display_options['style_options']['library'] = '';
  $handler->display->display_options['style_options']['label_field'] = 'genotype_description';
  $handler->display->display_options['style_options']['data_fields'] = array(
    'stock_id_1' => 'stock_id_1',
    'genotype_description' => 0,
  );
  $handler->display->display_options['style_options']['field_colors'] = array(
    'genotype_description' => '#2f7ed8',
    'stock_id_1' => '#0d233a',
  );
  $handler->display->display_options['style_options']['title'] = 'Genotype Distribution';
  $handler->display->display_options['style_options']['legend_position'] = 'bottom';
  $handler->display->display_options['style_options']['colors'] = array(
    0 => '#3c4a54',
    1 => '#a9b259',
    2 => '#406b48',
    3 => '#7c9a99',
    4 => '#413e57',
    5 => '#929000',
    6 => '#508020',
    7 => '#657f8b',
    8 => '#6a8053',
    9 => '#3a8080',
  );
  $handler->display->display_options['style_options']['width'] = '';
  $handler->display->display_options['style_options']['height'] = '';
  $handler->display->display_options['style_options']['xaxis_labels_rotation'] = '0';
  $handler->display->display_options['style_options']['yaxis_labels_rotation'] = '0';
  /* Field: Natural Diversity Genotype Experiments: Genotype Allele */
  $handler->display->display_options['fields']['genotype_description']['id'] = 'genotype_description';
  $handler->display->display_options['fields']['genotype_description']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['genotype_description']['field'] = 'genotype_description';
  $handler->display->display_options['fields']['genotype_description']['element_default_classes'] = FALSE;
  /* Field: COUNT(Natural Diversity Genotype Experiments: Stock ID) */
  $handler->display->display_options['fields']['stock_id_1']['id'] = 'stock_id_1';
  $handler->display->display_options['fields']['stock_id_1']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['stock_id_1']['field'] = 'stock_id';
  $handler->display->display_options['fields']['stock_id_1']['group_type'] = 'count';
  $handler->display->display_options['fields']['stock_id_1']['label'] = 'Germplasm';
  $handler->display->display_options['fields']['stock_id_1']['element_default_classes'] = FALSE;
  $handler->display->display_options['fields']['stock_id_1']['separator'] = '';
  /* Sort criterion: Natural Diversity Genotype Experiments: Genotype Allele */
  $handler->display->display_options['sorts']['genotype_description']['id'] = 'genotype_description';
  $handler->display->display_options['sorts']['genotype_description']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['sorts']['genotype_description']['field'] = 'genotype_description';
  /* Filter criterion: Natural Diversity Genotype Experiments: Genotype ID */
  $handler->display->display_options['filters']['genotype_id']['id'] = 'genotype_id';
  $handler->display->display_options['filters']['genotype_id']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['genotype_id']['field'] = 'genotype_id';
  $handler->display->display_options['filters']['genotype_id']['operator'] = '>';
  $handler->display->display_options['filters']['genotype_id']['value']['value'] = '0';
  /* Filter criterion: Natural Diversity Genotype Experiments: Feature ID */
  $handler->display->display_options['filters']['feature_id']['id'] = 'feature_id';
  $handler->display->display_options['filters']['feature_id']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['feature_id']['field'] = 'feature_id';
  $handler->display->display_options['filters']['feature_id']['value']['value'] = '355144';

  /* Display: Marker */
  $handler = $view->new_display('block', 'Marker', 'marker_block');

  return $view;
}

/**
 * Allows users to compare genotypes of germplasm of interest in a
 * user-specific marker by germplasm matrix.
 */
function nd_genotypes_genotype_matrix_defaultview() {

  $view = new view();
  $view->name = 'nd_genotypes_genotype_matrix';
  $view->description = 'Allows users to compare genotypes of germplasm of interest in a user-specific marker by germplasm matrix.';
  $view->tag = 'ND Genotypes';
  $view->base_table = 'feature';
  $view->human_name = 'ND Genotype Matrix';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Genotypes';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['group_by'] = TRUE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'input_required';
  $handler->display->display_options['exposed_form']['options']['submit_button'] = 'Search';
  $handler->display->display_options['exposed_form']['options']['text_input_required'] = 'You must enter at least one stock to display genotypes for under Genotypes above (highlighted in green). You will be able to add as many such genotype columns as you want during subsequent page loads.';
  $handler->display->display_options['exposed_form']['options']['text_input_required_format'] = 'full_html';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '25';
  $handler->display->display_options['pager']['options']['offset'] = '0';
  $handler->display->display_options['pager']['options']['id'] = '0';
  $handler->display->display_options['pager']['options']['quantity'] = '9';
  $handler->display->display_options['style_plugin'] = 'table';
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = 'Narrow down the markers displayed by setting any of the above options. If you would like to browse all markers in KnowPulse then click "Search" with the defaults selected.';
  $handler->display->display_options['empty']['area']['format'] = 'full_html';
  /* Relationship: Feature to ND Genotype Exp */
  $handler->display->display_options['relationships']['feature_id_to_nd_genotype_experiment']['id'] = 'feature_id_to_nd_genotype_experiment';
  $handler->display->display_options['relationships']['feature_id_to_nd_genotype_experiment']['table'] = 'feature';
  $handler->display->display_options['relationships']['feature_id_to_nd_genotype_experiment']['field'] = 'feature_id_to_nd_genotype_experiment';
  $handler->display->display_options['relationships']['feature_id_to_nd_genotype_experiment']['ui_name'] = 'Feature to ND Genotype Exp';
  $handler->display->display_options['relationships']['feature_id_to_nd_genotype_experiment']['label'] = 'Genotype Exp';
  /* Relationship: Feature to Feature Property */
  $handler->display->display_options['relationships']['feature_id_to_featureprop']['id'] = 'feature_id_to_featureprop';
  $handler->display->display_options['relationships']['feature_id_to_featureprop']['table'] = 'feature';
  $handler->display->display_options['relationships']['feature_id_to_featureprop']['field'] = 'feature_id_to_featureprop';
  $handler->display->display_options['relationships']['feature_id_to_featureprop']['ui_name'] = 'Feature to Feature Property';
  $handler->display->display_options['relationships']['feature_id_to_featureprop']['label'] = 'Marker Type';
  /* Field: NID */
  $handler->display->display_options['fields']['nid']['id'] = 'nid';
  $handler->display->display_options['fields']['nid']['table'] = 'node';
  $handler->display->display_options['fields']['nid']['field'] = 'nid';
  $handler->display->display_options['fields']['nid']['ui_name'] = 'NID';
  $handler->display->display_options['fields']['nid']['exclude'] = TRUE;
  /* Field: Node URL */
  $handler->display->display_options['fields']['nid_1']['id'] = 'nid_1';
  $handler->display->display_options['fields']['nid_1']['table'] = 'node';
  $handler->display->display_options['fields']['nid_1']['field'] = 'nid';
  $handler->display->display_options['fields']['nid_1']['ui_name'] = 'Node URL';
  $handler->display->display_options['fields']['nid_1']['label'] = '';
  $handler->display->display_options['fields']['nid_1']['exclude'] = TRUE;
  $handler->display->display_options['fields']['nid_1']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['nid_1']['alter']['text'] = 'node/[nid]';
  $handler->display->display_options['fields']['nid_1']['element_label_colon'] = FALSE;
  /* Field: Feature ID */
  $handler->display->display_options['fields']['feature_id']['id'] = 'feature_id';
  $handler->display->display_options['fields']['feature_id']['table'] = 'feature';
  $handler->display->display_options['fields']['feature_id']['field'] = 'feature_id';
  $handler->display->display_options['fields']['feature_id']['ui_name'] = 'Feature ID';
  $handler->display->display_options['fields']['feature_id']['exclude'] = TRUE;
  $handler->display->display_options['fields']['feature_id']['separator'] = '';
  /* Field: Feature Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'feature';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['ui_name'] = 'Feature Name';
  $handler->display->display_options['fields']['name']['label'] = 'Marker';
  $handler->display->display_options['fields']['name']['alter']['make_link'] = TRUE;
  $handler->display->display_options['fields']['name']['alter']['path'] = '[nid_1]';
  $handler->display->display_options['fields']['name']['alter']['target'] = '_blank';
  /* Field: Property Value */
  $handler->display->display_options['fields']['value']['id'] = 'value';
  $handler->display->display_options['fields']['value']['table'] = 'featureprop';
  $handler->display->display_options['fields']['value']['field'] = 'value';
  $handler->display->display_options['fields']['value']['relationship'] = 'feature_id_to_featureprop';
  $handler->display->display_options['fields']['value']['ui_name'] = 'Property Value';
  $handler->display->display_options['fields']['value']['label'] = 'Marker Type';
  /* Field: Organism Genus */
  $handler->display->display_options['fields']['genus']['id'] = 'genus';
  $handler->display->display_options['fields']['genus']['table'] = 'organism';
  $handler->display->display_options['fields']['genus']['field'] = 'genus';
  $handler->display->display_options['fields']['genus']['ui_name'] = 'Organism Genus';
  $handler->display->display_options['fields']['genus']['exclude'] = TRUE;
  /* Field: Organism Species */
  $handler->display->display_options['fields']['species']['id'] = 'species';
  $handler->display->display_options['fields']['species']['table'] = 'organism';
  $handler->display->display_options['fields']['species']['field'] = 'species';
  $handler->display->display_options['fields']['species']['ui_name'] = 'Organism Species';
  $handler->display->display_options['fields']['species']['label'] = 'Source Species';
  $handler->display->display_options['fields']['species']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['species']['alter']['text'] = '[genus] [species]';
  /* Field: ARRAY_AGG(Genotypes for Stock) */
  $handler->display->display_options['fields']['germplasm_genotype']['id'] = 'germplasm_genotype';
  $handler->display->display_options['fields']['germplasm_genotype']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['germplasm_genotype']['field'] = 'germplasm_genotype';
  $handler->display->display_options['fields']['germplasm_genotype']['relationship'] = 'feature_id_to_nd_genotype_experiment';
  $handler->display->display_options['fields']['germplasm_genotype']['group_type'] = 'array_agg';
  $handler->display->display_options['fields']['germplasm_genotype']['ui_name'] = 'Genotypes for Stock';
  $handler->display->display_options['fields']['germplasm_genotype']['label'] = 'Genotypes';
  $handler->display->display_options['fields']['germplasm_genotype']['exclude'] = TRUE;
  $handler->display->display_options['fields']['germplasm_genotype']['element_class'] = 'views-germplasm-genotype genotype-[germplasm_genotype] short-column';
  $handler->display->display_options['fields']['germplasm_genotype']['element_label_class'] = 'short-column';
  $handler->display->display_options['fields']['germplasm_genotype']['settings'] = array(
    'germplasm' => array(
      'stock_name' => '',
      'organism_id' => '<ANY>',
    ),
    'use_filter' => '<NONE>',
    'has_genotype' => 1,
    'get_selections' => 1,
  );
  $handler->display->display_options['fields']['germplasm_genotype']['render'] = array(
    'render_type' => 'both',
    'color_bases' => 1,
  );
  /* Sort criterion: Chado Organism: Genus */
  $handler->display->display_options['sorts']['genus']['id'] = 'genus';
  $handler->display->display_options['sorts']['genus']['table'] = 'organism';
  $handler->display->display_options['sorts']['genus']['field'] = 'genus';
  /* Sort criterion: Chado Feature: Name */
  $handler->display->display_options['sorts']['name']['id'] = 'name';
  $handler->display->display_options['sorts']['name']['table'] = 'feature';
  $handler->display->display_options['sorts']['name']['field'] = 'name';
  /* Contextual filter: Chado Organism: Genus */
  $handler->display->display_options['arguments']['genus']['id'] = 'genus';
  $handler->display->display_options['arguments']['genus']['table'] = 'organism';
  $handler->display->display_options['arguments']['genus']['field'] = 'genus';
  $handler->display->display_options['arguments']['genus']['title_enable'] = TRUE;
  $handler->display->display_options['arguments']['genus']['title'] = 'Genotypes for %1 Species';
  $handler->display->display_options['arguments']['genus']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['genus']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['genus']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['genus']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['genus']['limit'] = '0';
  /* Filter criterion: Feature Type ID */
  $handler->display->display_options['filters']['type_id_1']['id'] = 'type_id_1';
  $handler->display->display_options['filters']['type_id_1']['table'] = 'feature';
  $handler->display->display_options['filters']['type_id_1']['field'] = 'type_id';
  $handler->display->display_options['filters']['type_id_1']['ui_name'] = 'Feature Type ID';
  $handler->display->display_options['filters']['type_id_1']['value'] = array(
    3686 => '3686',
  );
  $handler->display->display_options['filters']['type_id_1']['group'] = 1;
  $handler->display->display_options['filters']['type_id_1']['values_form_type'] = 'select';
  $handler->display->display_options['filters']['type_id_1']['select_multiple'] = TRUE;
  $handler->display->display_options['filters']['type_id_1']['max_length'] = '40';
  /* Filter criterion: Property Type ID */
  $handler->display->display_options['filters']['type_id']['id'] = 'type_id';
  $handler->display->display_options['filters']['type_id']['table'] = 'featureprop';
  $handler->display->display_options['filters']['type_id']['field'] = 'type_id';
  $handler->display->display_options['filters']['type_id']['relationship'] = 'feature_id_to_featureprop';
  $handler->display->display_options['filters']['type_id']['ui_name'] = 'Property Type ID';
  $handler->display->display_options['filters']['type_id']['value'] = '3966';
  $handler->display->display_options['filters']['type_id']['group'] = 1;
  $handler->display->display_options['filters']['type_id']['values_form_type'] = 'select';
  $handler->display->display_options['filters']['type_id']['max_length'] = '';
  /* Filter criterion: Genotypes for Stocks+ */
  $handler->display->display_options['filters']['add_germplasm_genotype']['id'] = 'add_germplasm_genotype';
  $handler->display->display_options['filters']['add_germplasm_genotype']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['add_germplasm_genotype']['field'] = 'add_germplasm_genotype';
  $handler->display->display_options['filters']['add_germplasm_genotype']['relationship'] = 'feature_id_to_nd_genotype_experiment';
  $handler->display->display_options['filters']['add_germplasm_genotype']['group_type'] = 'array_agg';
  $handler->display->display_options['filters']['add_germplasm_genotype']['ui_name'] = 'Genotypes for Stocks+';
  $handler->display->display_options['filters']['add_germplasm_genotype']['value'] = array(
    'add_stock' => array(
      'stock_name' => '',
      'organism_id' => '<ANY>',
    ),
  );
  $handler->display->display_options['filters']['add_germplasm_genotype']['group'] = 1;
  $handler->display->display_options['filters']['add_germplasm_genotype']['exposed'] = TRUE;
  $handler->display->display_options['filters']['add_germplasm_genotype']['expose']['operator_id'] = 'add_germplasm_genotype_op';
  $handler->display->display_options['filters']['add_germplasm_genotype']['expose']['label'] = 'Genotypes';
  $handler->display->display_options['filters']['add_germplasm_genotype']['expose']['description'] = 'Specify the stock (and species of the stock) you want to display the genotypes of.';
  $handler->display->display_options['filters']['add_germplasm_genotype']['expose']['operator'] = 'add_germplasm_genotype_op';
  $handler->display->display_options['filters']['add_germplasm_genotype']['expose']['identifier'] = 'add_germplasm_genotype';
  $handler->display->display_options['filters']['add_germplasm_genotype']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  /* Filter criterion: Polymorphic Markers */
  $handler->display->display_options['filters']['polymorphic_germplasm']['id'] = 'polymorphic_germplasm';
  $handler->display->display_options['filters']['polymorphic_germplasm']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['polymorphic_germplasm']['field'] = 'polymorphic_germplasm';
  $handler->display->display_options['filters']['polymorphic_germplasm']['relationship'] = 'feature_id_to_nd_genotype_experiment';
  $handler->display->display_options['filters']['polymorphic_germplasm']['group_type'] = 'array_agg';
  $handler->display->display_options['filters']['polymorphic_germplasm']['ui_name'] = 'Polymorphic Markers';
  $handler->display->display_options['filters']['polymorphic_germplasm']['value'] = array(
    'first' => '<NONE>',
    'second' => '<NONE>',
  );
  $handler->display->display_options['filters']['polymorphic_germplasm']['group'] = 1;
  $handler->display->display_options['filters']['polymorphic_germplasm']['exposed'] = TRUE;
  $handler->display->display_options['filters']['polymorphic_germplasm']['expose']['operator_id'] = 'polymorphic_germplasm_op';
  $handler->display->display_options['filters']['polymorphic_germplasm']['expose']['label'] = 'Polymorphic Markers';
  $handler->display->display_options['filters']['polymorphic_germplasm']['expose']['description'] = 'Display only markers which are polymorphic (different) between the two indicated stocks. This filter is populated with the stocks added in the "Genotypes" filter and thus will only become useful once you have added at least two stocks.';
  $handler->display->display_options['filters']['polymorphic_germplasm']['expose']['operator'] = 'polymorphic_germplasm_op';
  $handler->display->display_options['filters']['polymorphic_germplasm']['expose']['identifier'] = 'polymorphic_germplasm';
  $handler->display->display_options['filters']['polymorphic_germplasm']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  /* Filter criterion: Organism Species */
  $handler->display->display_options['filters']['species']['id'] = 'species';
  $handler->display->display_options['filters']['species']['table'] = 'organism';
  $handler->display->display_options['filters']['species']['field'] = 'species';
  $handler->display->display_options['filters']['species']['ui_name'] = 'Organism Species';
  $handler->display->display_options['filters']['species']['group'] = 1;
  $handler->display->display_options['filters']['species']['exposed'] = TRUE;
  $handler->display->display_options['filters']['species']['expose']['operator_id'] = 'species_op';
  $handler->display->display_options['filters']['species']['expose']['label'] = 'Species';
  $handler->display->display_options['filters']['species']['expose']['description'] = 'The organism species the marker was developed for';
  $handler->display->display_options['filters']['species']['expose']['operator'] = 'species_op';
  $handler->display->display_options['filters']['species']['expose']['identifier'] = 'species';
  $handler->display->display_options['filters']['species']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  $handler->display->display_options['filters']['species']['values_form_type'] = 'select';
  $handler->display->display_options['filters']['species']['select_multiple'] = TRUE;
  $handler->display->display_options['filters']['species']['select_optional'] = TRUE;
  $handler->display->display_options['filters']['species']['max_length'] = '40';
  /* Filter criterion: Property Value */
  $handler->display->display_options['filters']['value']['id'] = 'value';
  $handler->display->display_options['filters']['value']['table'] = 'featureprop';
  $handler->display->display_options['filters']['value']['field'] = 'value';
  $handler->display->display_options['filters']['value']['relationship'] = 'feature_id_to_featureprop';
  $handler->display->display_options['filters']['value']['ui_name'] = 'Property Value';
  $handler->display->display_options['filters']['value']['group'] = 1;
  $handler->display->display_options['filters']['value']['exposed'] = TRUE;
  $handler->display->display_options['filters']['value']['expose']['operator_id'] = 'value_op';
  $handler->display->display_options['filters']['value']['expose']['label'] = 'Marker Type';
  $handler->display->display_options['filters']['value']['expose']['description'] = 'The type of marker you are interested in';
  $handler->display->display_options['filters']['value']['expose']['operator'] = 'value_op';
  $handler->display->display_options['filters']['value']['expose']['identifier'] = 'marker_type';
  $handler->display->display_options['filters']['value']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  $handler->display->display_options['filters']['value']['values_form_type'] = 'select';
  $handler->display->display_options['filters']['value']['select_multiple'] = TRUE;
  $handler->display->display_options['filters']['value']['select_optional'] = TRUE;
  $handler->display->display_options['filters']['value']['max_length'] = '40';
  /* Filter criterion: Natural Diversity Genotype Experiments: Project ID */
  $handler->display->display_options['filters']['project_id']['id'] = 'project_id';
  $handler->display->display_options['filters']['project_id']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['project_id']['field'] = 'project_id';
  $handler->display->display_options['filters']['project_id']['relationship'] = 'feature_id_to_nd_genotype_experiment';
  $handler->display->display_options['filters']['project_id']['exposed'] = TRUE;
  $handler->display->display_options['filters']['project_id']['expose']['operator_id'] = 'project_id_op';
  $handler->display->display_options['filters']['project_id']['expose']['label'] = 'Project';
  $handler->display->display_options['filters']['project_id']['expose']['description'] = 'The project the genotypes you are interested in where generated as part of';
  $handler->display->display_options['filters']['project_id']['expose']['operator'] = 'project_id_op';
  $handler->display->display_options['filters']['project_id']['expose']['identifier'] = 'project_id';
  $handler->display->display_options['filters']['project_id']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    4 => 0,
    5 => 0,
    3 => 0,
  );
  $handler->display->display_options['filters']['project_id']['values_form_type'] = 'select';
  $handler->display->display_options['filters']['project_id']['select_optional'] = TRUE;
  $handler->display->display_options['filters']['project_id']['max_length'] = '40';
  /* Filter criterion: Feature Name */
  $handler->display->display_options['filters']['name']['id'] = 'name';
  $handler->display->display_options['filters']['name']['table'] = 'feature';
  $handler->display->display_options['filters']['name']['field'] = 'name';
  $handler->display->display_options['filters']['name']['ui_name'] = 'Feature Name';
  $handler->display->display_options['filters']['name']['operator'] = 'contains';
  $handler->display->display_options['filters']['name']['group'] = 1;
  $handler->display->display_options['filters']['name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['name']['expose']['operator_id'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['label'] = 'Marker Name';
  $handler->display->display_options['filters']['name']['expose']['description'] = 'The name of the marker (partial names are accepted)';
  $handler->display->display_options['filters']['name']['expose']['operator'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['identifier'] = 'name';
  $handler->display->display_options['filters']['name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  $handler->display->display_options['filters']['name']['max_length'] = '';

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'genotype_matrix/%';

  return $view;
}

/**
 * Lists Germplasm genotypes for a particular organism.
 * Used in conjuction with the genotype matrix.
 */
function nd_genotypes_genotype_matrix_germplasm_list_defaultview() {

  $view = new view();
  $view->name = 'nd_genotypes_genotype_matrix_germplasm_list';
  $view->description = 'A list of the germplasm genotyped to be linked to from the genotype matrix page';
  $view->tag = 'ND Genotypes';
  $view->base_table = 'nd_genotype_germplasm';
  $view->human_name = 'ND Genotype Matrix: Germplasm Genotyped List';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Germplasm Genotyped';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '50';
  $handler->display->display_options['style_plugin'] = 'table';
  /* Relationship: Chado Nd Genotype Germplasm: stock_id => Stock */
  $handler->display->display_options['relationships']['stock_id_to_stock']['id'] = 'stock_id_to_stock';
  $handler->display->display_options['relationships']['stock_id_to_stock']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['relationships']['stock_id_to_stock']['field'] = 'stock_id_to_stock';
  $handler->display->display_options['relationships']['stock_id_to_stock']['label'] = 'Base to Stock';
  /* Relationship: Stock: Stock => Node */
  $handler->display->display_options['relationships']['stock_id']['id'] = 'stock_id';
  $handler->display->display_options['relationships']['stock_id']['table'] = 'chado_stock';
  $handler->display->display_options['relationships']['stock_id']['field'] = 'stock_id';
  $handler->display->display_options['relationships']['stock_id']['relationship'] = 'stock_id_to_stock';
  $handler->display->display_options['relationships']['stock_id']['label'] = 'Stock to Node';
  /* Field: Node URL */
  $handler->display->display_options['fields']['nid']['id'] = 'nid';
  $handler->display->display_options['fields']['nid']['table'] = 'node';
  $handler->display->display_options['fields']['nid']['field'] = 'nid';
  $handler->display->display_options['fields']['nid']['relationship'] = 'stock_id';
  $handler->display->display_options['fields']['nid']['ui_name'] = 'Node URL';
  $handler->display->display_options['fields']['nid']['label'] = '';
  $handler->display->display_options['fields']['nid']['exclude'] = TRUE;
  $handler->display->display_options['fields']['nid']['alter']['alter_text'] = TRUE;
  $handler->display->display_options['fields']['nid']['alter']['text'] = 'node/[nid]';
  $handler->display->display_options['fields']['nid']['element_label_colon'] = FALSE;
  /* Field: Chado Nd Genotype Germplasm: Name */
  $handler->display->display_options['fields']['name']['id'] = 'name';
  $handler->display->display_options['fields']['name']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['fields']['name']['field'] = 'name';
  $handler->display->display_options['fields']['name']['alter']['make_link'] = TRUE;
  $handler->display->display_options['fields']['name']['alter']['path'] = '[nid]';
  /* Field: Chado Nd Genotype Germplasm: Uniquename */
  $handler->display->display_options['fields']['uniquename']['id'] = 'uniquename';
  $handler->display->display_options['fields']['uniquename']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['fields']['uniquename']['field'] = 'uniquename';
  $handler->display->display_options['fields']['uniquename']['label'] = 'Unique Name';
  /* Field: Chado Nd Genotype Germplasm: Species */
  $handler->display->display_options['fields']['species']['id'] = 'species';
  $handler->display->display_options['fields']['species']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['fields']['species']['field'] = 'species';
  $handler->display->display_options['fields']['species']['label'] = 'Source Species';
  /* Field: Chado Nd Genotype Germplasm: Num Genotypes */
  $handler->display->display_options['fields']['num_genotypes']['id'] = 'num_genotypes';
  $handler->display->display_options['fields']['num_genotypes']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['fields']['num_genotypes']['field'] = 'num_genotypes';
  $handler->display->display_options['fields']['num_genotypes']['label'] = 'Number of Genotypes Available';
  $handler->display->display_options['fields']['num_genotypes']['separator'] = '';
  /* Sort criterion: Chado Nd Genotype Germplasm: Species */
  $handler->display->display_options['sorts']['species']['id'] = 'species';
  $handler->display->display_options['sorts']['species']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['sorts']['species']['field'] = 'species';
  /* Sort criterion: Chado Nd Genotype Germplasm: Name */
  $handler->display->display_options['sorts']['name']['id'] = 'name';
  $handler->display->display_options['sorts']['name']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['sorts']['name']['field'] = 'name';
  /* Contextual filter: Chado Nd Genotype Germplasm: Genus */
  $handler->display->display_options['arguments']['genus']['id'] = 'genus';
  $handler->display->display_options['arguments']['genus']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['arguments']['genus']['field'] = 'genus';
  $handler->display->display_options['arguments']['genus']['title_enable'] = TRUE;
  $handler->display->display_options['arguments']['genus']['title'] = '%1 Germplasm Genotyped';
  $handler->display->display_options['arguments']['genus']['default_argument_type'] = 'fixed';
  $handler->display->display_options['arguments']['genus']['summary']['number_of_records'] = '0';
  $handler->display->display_options['arguments']['genus']['summary']['format'] = 'default_summary';
  $handler->display->display_options['arguments']['genus']['summary_options']['items_per_page'] = '25';
  $handler->display->display_options['arguments']['genus']['limit'] = '0';
  /* Filter criterion: Chado Nd Genotype Germplasm: Species */
  $handler->display->display_options['filters']['species']['id'] = 'species';
  $handler->display->display_options['filters']['species']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['filters']['species']['field'] = 'species';
  $handler->display->display_options['filters']['species']['value'] = array();
  $handler->display->display_options['filters']['species']['exposed'] = TRUE;
  $handler->display->display_options['filters']['species']['expose']['operator_id'] = 'species_op';
  $handler->display->display_options['filters']['species']['expose']['label'] = 'Source Species';
  $handler->display->display_options['filters']['species']['expose']['description'] = 'The species of the germplasm you are interested in';
  $handler->display->display_options['filters']['species']['expose']['operator'] = 'species_op';
  $handler->display->display_options['filters']['species']['expose']['identifier'] = 'species';
  $handler->display->display_options['filters']['species']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  $handler->display->display_options['filters']['species']['values_form_type'] = 'select';
  $handler->display->display_options['filters']['species']['select_multiple'] = TRUE;
  $handler->display->display_options['filters']['species']['select_optional'] = TRUE;
  $handler->display->display_options['filters']['species']['max_length'] = '40';
  /* Filter criterion: Chado Nd Genotype Germplasm: Name */
  $handler->display->display_options['filters']['name']['id'] = 'name';
  $handler->display->display_options['filters']['name']['table'] = 'nd_genotype_germplasm';
  $handler->display->display_options['filters']['name']['field'] = 'name';
  $handler->display->display_options['filters']['name']['operator'] = 'contains';
  $handler->display->display_options['filters']['name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['name']['expose']['operator_id'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['label'] = 'Name';
  $handler->display->display_options['filters']['name']['expose']['description'] = 'The name of the germplasm (partial names are accepted)';
  $handler->display->display_options['filters']['name']['expose']['operator'] = 'name_op';
  $handler->display->display_options['filters']['name']['expose']['identifier'] = 'name';
  $handler->display->display_options['filters']['name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    3 => 0,
  );
  $handler->display->display_options['filters']['name']['max_length'] = '40';

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'genotype_matrix/%/germplasm';

  return $view;
}

/**
 * Genotype administrative view
 */
function nd_genotypes_genotype_admin_defaultview() {

  $view = new view();
  $view->name = 'nd_genotype_experiments_admin';
  $view->description = '';
  $view->tag = 'ND Genotypes';
  $view->base_table = 'nd_genotype_experiment';
  $view->human_name = 'Genotype Experiments Admin';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'Genotype Experiments';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['pager']['type'] = 'full';
  $handler->display->display_options['pager']['options']['items_per_page'] = '50';
  $handler->display->display_options['style_plugin'] = 'table';
  /* Header: Global: Action Links */
  $handler->display->display_options['header']['action_links_area']['id'] = 'action_links_area';
  $handler->display->display_options['header']['action_links_area']['table'] = 'views';
  $handler->display->display_options['header']['action_links_area']['field'] = 'action_links_area';
  $handler->display->display_options['header']['action_links_area']['label'] = 'Action Links';
  $handler->display->display_options['header']['action_links_area']['empty'] = TRUE;
  $handler->display->display_options['header']['action_links_area']['link-1'] = array(
    'label-1' => 'Import via Bulk Loader',
    'path-1' => 'node/add/tripal-bulk-loader',
  );
  /* No results behavior: Global: Text area */
  $handler->display->display_options['empty']['area']['id'] = 'area';
  $handler->display->display_options['empty']['area']['table'] = 'views';
  $handler->display->display_options['empty']['area']['field'] = 'area';
  $handler->display->display_options['empty']['area']['label'] = 'No Genotypes';
  $handler->display->display_options['empty']['area']['empty'] = TRUE;
  $handler->display->display_options['empty']['area']['content'] = 'There are no genotypes currently sync\'d through the ND Genotypes module. To get data to appear here and be available to the ND Genotype module you need to
  <ol>
  <li>Load genotypic data such that this module can decipher it. The easiest way to do this is to use the Tripal Bulk Loading template, "ND Genotype: Generic Genotype Experiments", provided with this module. See the Sync tab to re-add the template if you don\'t have it or need to revert any changes.</li>
  <li>Sync that data by populating the nd_genotype_experiment materialized view. This should be done through the Sync tab above.</li>
  </ol>';
  $handler->display->display_options['empty']['area']['format'] = 'full_html';
  /* Field: Natural Diversity Genotype Experiments: ND Experiment ID */
  $handler->display->display_options['fields']['nd_experiment_id']['id'] = 'nd_experiment_id';
  $handler->display->display_options['fields']['nd_experiment_id']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['nd_experiment_id']['field'] = 'nd_experiment_id';
  $handler->display->display_options['fields']['nd_experiment_id']['separator'] = '';
  /* Field: Natural Diversity Genotype Experiments: Stock Name */
  $handler->display->display_options['fields']['stock_name']['id'] = 'stock_name';
  $handler->display->display_options['fields']['stock_name']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['stock_name']['field'] = 'stock_name';
  $handler->display->display_options['fields']['stock_name']['label'] = 'Stock';
  /* Field: Natural Diversity Genotype Experiments: Feature Name */
  $handler->display->display_options['fields']['feature_name']['id'] = 'feature_name';
  $handler->display->display_options['fields']['feature_name']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['feature_name']['field'] = 'feature_name';
  $handler->display->display_options['fields']['feature_name']['label'] = 'Feature';
  /* Field: Natural Diversity Genotype Experiments: Genotype Allele */
  $handler->display->display_options['fields']['genotype_description']['id'] = 'genotype_description';
  $handler->display->display_options['fields']['genotype_description']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['genotype_description']['field'] = 'genotype_description';
  /* Field: Natural Diversity Genotype Experiments: Project Name */
  $handler->display->display_options['fields']['project_name']['id'] = 'project_name';
  $handler->display->display_options['fields']['project_name']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['fields']['project_name']['field'] = 'project_name';
  $handler->display->display_options['fields']['project_name']['label'] = 'Project';
  /* Filter criterion: Project */
  $handler->display->display_options['filters']['project_id']['id'] = 'project_id';
  $handler->display->display_options['filters']['project_id']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['project_id']['field'] = 'project_id';
  $handler->display->display_options['filters']['project_id']['ui_name'] = 'Project';
  $handler->display->display_options['filters']['project_id']['value'] = array();
  $handler->display->display_options['filters']['project_id']['exposed'] = TRUE;
  $handler->display->display_options['filters']['project_id']['expose']['operator_id'] = 'project_id_op';
  $handler->display->display_options['filters']['project_id']['expose']['label'] = 'Project';
  $handler->display->display_options['filters']['project_id']['expose']['description'] = 'The project the experiment is associated with';
  $handler->display->display_options['filters']['project_id']['expose']['operator'] = 'project_id_op';
  $handler->display->display_options['filters']['project_id']['expose']['identifier'] = 'project_id';
  $handler->display->display_options['filters']['project_id']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    4 => 0,
    5 => 0,
    3 => 0,
    6 => 0,
  );
  $handler->display->display_options['filters']['project_id']['values_form_type'] = 'select';
  $handler->display->display_options['filters']['project_id']['select_multiple'] = TRUE;
  $handler->display->display_options['filters']['project_id']['max_length'] = '40';
  /* Filter criterion: Natural Diversity Genotype Experiments: Feature Name */
  $handler->display->display_options['filters']['feature_name']['id'] = 'feature_name';
  $handler->display->display_options['filters']['feature_name']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['feature_name']['field'] = 'feature_name';
  $handler->display->display_options['filters']['feature_name']['operator'] = 'contains';
  $handler->display->display_options['filters']['feature_name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['feature_name']['expose']['operator_id'] = 'feature_name_op';
  $handler->display->display_options['filters']['feature_name']['expose']['label'] = 'Feature Name Contains';
  $handler->display->display_options['filters']['feature_name']['expose']['description'] = 'The name of the feature the genotypes are associated with (partial names accepted)';
  $handler->display->display_options['filters']['feature_name']['expose']['operator'] = 'feature_name_op';
  $handler->display->display_options['filters']['feature_name']['expose']['identifier'] = 'feature_name';
  $handler->display->display_options['filters']['feature_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    4 => 0,
    5 => 0,
    3 => 0,
    6 => 0,
  );
  /* Filter criterion: Natural Diversity Genotype Experiments: Stock Name */
  $handler->display->display_options['filters']['stock_name']['id'] = 'stock_name';
  $handler->display->display_options['filters']['stock_name']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['filters']['stock_name']['field'] = 'stock_name';
  $handler->display->display_options['filters']['stock_name']['operator'] = 'contains';
  $handler->display->display_options['filters']['stock_name']['exposed'] = TRUE;
  $handler->display->display_options['filters']['stock_name']['expose']['operator_id'] = 'stock_name_op';
  $handler->display->display_options['filters']['stock_name']['expose']['label'] = 'Stock Name Contains';
  $handler->display->display_options['filters']['stock_name']['expose']['description'] = 'The name of the stock the genotypes are associated with (partial names accepted)';
  $handler->display->display_options['filters']['stock_name']['expose']['operator'] = 'stock_name_op';
  $handler->display->display_options['filters']['stock_name']['expose']['identifier'] = 'stock_name';
  $handler->display->display_options['filters']['stock_name']['expose']['remember_roles'] = array(
    2 => '2',
    1 => 0,
    4 => 0,
    5 => 0,
    3 => 0,
    6 => 0,
  );
  /* Sort criterion: Natural Diversity Genotype Experiments: Feature Name */
  $handler->display->display_options['sorts']['feature_name']['id'] = 'feature_name';
  $handler->display->display_options['sorts']['feature_name']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['sorts']['feature_name']['field'] = 'feature_name';
  /* Sort criterion: Natural Diversity Genotype Experiments: Stock Name */
  $handler->display->display_options['sorts']['stock_name']['id'] = 'stock_name';
  $handler->display->display_options['sorts']['stock_name']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['sorts']['stock_name']['field'] = 'stock_name';
  /* Sort criterion: Natural Diversity Genotype Experiments: Genotype Allele */
  $handler->display->display_options['sorts']['genotype_description']['id'] = 'genotype_description';
  $handler->display->display_options['sorts']['genotype_description']['table'] = 'nd_genotype_experiment';
  $handler->display->display_options['sorts']['genotype_description']['field'] = 'genotype_description';

  /* Display: Page */
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->display->display_options['path'] = 'admin/tripal/extension/nd_genotypes/genotypes';
  $handler->display->display_options['menu']['type'] = 'default tab';
  $handler->display->display_options['menu']['title'] = 'Genotypes';
  $handler->display->display_options['menu']['weight'] = '0';
  $handler->display->display_options['menu']['context'] = 0;
  $handler->display->display_options['menu']['context_only_inline'] = 0;
  $handler->display->display_options['tab_options']['weight'] = '0';

  return $view;
}