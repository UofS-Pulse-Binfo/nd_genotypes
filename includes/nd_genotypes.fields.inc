<?php
/**
 * @file
 * Contains hooks related to fields provided by this module.
 */

/**
 * Implements hook_bundle_fields_info().
 *
 * @param $entity_type
 *   This should be 'TripalEntity' for all Tripal Content.
 * @param $bundle
 *   This object describes the Type of Tripal Entity (e.g. Organism or Gene) this hook is
 *   being called for. However, since this hook creates field types (by definition not
 *   tied to a specific Tripal Content Type (bundle)) and since a field type will only be
 *   created if it doesn't already exist, this parameter doesn't actually matter.
 *   NOTE: If you do need to determine the bundle in this hook, we suggest inspecting
 *   the data_table since the label can be changed by site administrators.
 *
 * @return
 *   An array of field definitions. Each field in this array will be created if it
 *   doesn't already exist. To trigger create of fields when developing call
 *   tripal_refresh_bundle_fields() for the specific bundle.
 */
function nd_genotypes_bundle_fields_info($entity_type, $bundle) {
  $fields = array();

  // ANY CHADO-BASED CONTENT TYPES.
  /////////////////////////////////////
  if (isset($bundle->data_table)) {
    // Genotype Matrix Link.
    //---------------------------------
    // First add my term.
    tripal_insert_cvterm(array(
      'id' => 'local:genotype_matrix_link',
      'name' => 'genotype_matrix_link',
      'cv_name' => 'local',
      'definition' => 'A link to a pre-filtered genotype matrix.',
    ));

    // Then describe the field defined by that term.
    $field_name = 'local__genotype_matrix_link';
    $field_type = 'local__genotype_matrix_link';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      // Controlls how many values you expect your field to have.
      'cardinality' => 0,
      // This locks your field so it can't be deleted by an administrator --use with care!
      'locked' => FALSE,
      // Indicated the storage API this field uses.
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );
  }

  // STOCK-BASED CONTENT TYPES.
  /////////////////////////////////////
  if (isset($bundle->data_table) AND ($bundle->data_table == 'stock')) {

    // Magnitude of Genotypes.
    //---------------------------------
    tripal_insert_cvterm(array(
      'id' => 'local:germ_marker_summary',
      'name' => 'germ_marker_summary',
      'cv_name' => 'local',
      'definition' => 'Germplasm Marker Summary',
    ));

    $field_name = 'local__germ_marker_summary';
    $field_type = 'local__germ_marker_summary';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      // Controlls how many values you expect your field to have.
      'cardinality' => 0,
      // This locks your field so it can't be deleted by an administrator --use with care!
      'locked' => FALSE,
      // Indicated the storage API this field uses.
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );
  }

  // FEATURE-BASED CONTENT TYPES.
  /////////////////////////////////////
  if (isset($bundle->data_table) AND ($bundle->data_table == 'feature')) {

    // Variant Genotype Summary Field.
    //---------------------------------
    // First add my term.
    tripal_insert_cvterm(array(
      'id' => 'local:variant_genotype_summary',
      'name' => 'variant_genotype_summary',
      'cv_name' => 'local',
      'definition' => 'A summary of genotypic data for a given variant.',
    ));

    // Then describe the field defined by that term.
    $field_name = 'local__variant_genotype_summary';
    $field_type = 'local__variant_genotype_summary';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      // Controlls how many values you expect your field to have.
      'cardinality' => 0,
      // This locks your field so it can't be deleted by an administrator --use with care!
      'locked' => FALSE,
      // Indicated the storage API this field uses.
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );

    // Marker Genotype Summary Field.
    //---------------------------------
    // First add my term.
    tripal_insert_cvterm(array(
      'id' => 'local:marker_genotype_summary',
      'name' => 'marker_genotype_summary',
      'cv_name' => 'local',
      'definition' => 'A summary of genotypic data for a given marker.',
    ));

    // Then describe the field defined by that term.
    $field_name = 'local__marker_genotype_summary';
    $field_type = 'local__marker_genotype_summary';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      // Controlls how many values you expect your field to have.
      'cardinality' => 1,
      // This locks your field so it can't be deleted by an administrator --use with care!
      'locked' => FALSE,
      // Indicated the storage API this field uses.
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );

    // Marker List Field.
    //----------------------
    // First add my term.
    tripal_insert_cvterm(array(
      'id' => 'SO:0001645',
      'name' => 'genetic_marker',
      'cv_name' => 'sequence',
      'definition' => 'A measurable sequence feature that varies within a population. [SO:db]',
    ));

    // Then describe the field defined by that term.
    $field_name = 'so__genetic_marker';
    $field_type = 'so__genetic_marker';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );

    // Sequence with Variants Field.
    //---------------------------------
    // First add my term.
    tripal_insert_cvterm(array(
      'id' => 'local:sequence_with_variants',
      'name' => 'sequence_with_variants',
      'cv_name' => 'local',
      'definition' => 'Flanking sequence for a given variant showing all known flanking variants.',
    ));

    // We also need the following term to describe the value of this field.
    tripal_insert_cvterm(array(
      'id' => 'local:variant_coordinates',
      'name' => 'variant_coordinates',
      'cv_name' => 'local',
      'definition' => 'The coordinates of the associated variant.',
    ));

    // Then describe the field defined by that term.
    $field_name = 'local__sequence_with_variants';
    $field_type = 'local__sequence_with_variants';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      // Controlls how many values you expect your field to have.
      'cardinality' => 0,
      // This locks your field so it can't be deleted by an administrator --use with care!
      'locked' => FALSE,
      // Indicated the storage API this field uses.
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );
  }

  return $fields;
}

/**
 * Impelments hook_create_tripalfield_instance().
 *
 * This is a Tripal defined hook that supports integration with the
 * TripalEntity field.
 */
function nd_genotypes_bundle_instances_info($entity_type, $bundle) {
  $info = array();

  // Load the term used for the content type.
  $term = tripal_load_term_entity(array('term_id' => $bundle->term_id));

  // STOCK-BASED CONTENT TYPES.
  /////////////////////////////////////
  if (isset($bundle->data_table) AND ($bundle->data_table == 'stock')) {

    // Magnitude of Genotypes.
    //---------------------------------
    $info['local__germ_marker_summary'] =  array(
      'field_name' => 'local__germ_marker_summary',
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Germplasm Marker Summary',
      'description' => 'Summarizes the markers associated with a given germplasm.',
      'required' => FALSE,
      'settings' => array(
        'auto_attach' => FALSE,
        'term_vocabulary' => 'local',
        'term_accession' => 'germ_marker_summary',
        'term_name' => 'germ_marker_summary',
      ),
      'widget' => array(
        'type' => 'local__germ_marker_summary_widget',
        'settings' => array(
          'display_label' => 1,
        ),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'local__germ_marker_summary_formatter',
          'settings' => array(),
        ),
      ),
    );
  }

  // FEATURE-BASED CONTENT TYPES.
  /////////////////////////////////////
  if (isset($bundle->data_table) AND ($bundle->data_table == 'feature')) {

    // Variant Genotype Summary Field.
    //---------------------------------
    // Only add the Variant Genotype Summary field
    // if the current Tripal Content Type is SO:sequence_variant.
    if ($term->name == 'sequence_variant') {

      $info['local__variant_genotype_summary'] =  array(
        'field_name' => 'local__variant_genotype_summary',
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Genotype Summary',
        'description' => 'Summarizes the genotypes associated with a given variant.',
        'required' => FALSE,
        'settings' => array(
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => 'feature_id',
          'base_table' => $bundle->data_table,
          'term_vocabulary' => 'local',
          'term_accession' => 'variant_genotype_summary',
          'term_name' => 'variant_genotype_summary',
        ),
        'widget' => array(
          'type' => 'local__variant_genotype_summary_widget',
          'settings' => array(
            'display_label' => 1,
          ),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'local__variant_genotype_summary_formatter',
            'settings' => array(),
          ),
        ),
      );

      // Genetic Marker List
      //----------------------
      $field_name = 'so__genetic_marker';
      $field_type = 'so__genetic_marker';
      $instances[$field_name] =  array(
        'field_name' => $field_name,
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Genetic Marker List',
        'description' => 'Lists genetic markers for a given sequence variant.',
        'required' => FALSE,
        'settings' => array(
          'term_vocabulary' => 'SO',
          'term_name' => 'genetic_marker',
          'term_accession' => '0001645',
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => 'organism_id',
          'base_table' => $bundle->data_table,
        ),
        'widget' => array(
          'type' => 'so__genetic_marker_widget',
          'settings' => array(),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'so__genetic_marker_formatter',
            'settings' => array(),
          ),
        ),
      );
    }
    // Marker Genotype Summary Field.
    //---------------------------------
    // Only add the Marker Genotype Summary field
    // if the current Tripal Content Type is SO:genetic_marker.
    elseif ($term->name == 'genetic_marker') {

      $info['local__marker_genotype_summary'] =  array(
        'field_name' => 'local__marker_genotype_summary',
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Genotype Summary',
        'description' => 'Summarizes the genotypes associated with a given marker.',
        'required' => FALSE,
        'settings' => array(
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => 'feature_id',
          'base_table' => $bundle->data_table,
          'term_vocabulary' => 'local',
          'term_accession' => 'marker_genotype_summary',
          'term_name' => 'marker_genotype_summary',
        ),
        'widget' => array(
          'type' => 'local__marker_genotype_summary_widget',
          'settings' => array(
            'display_label' => 1,
          ),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'local__marker_genotype_summary_formatter',
            'settings' => array(),
          ),
        ),
      );
    }

    // Add to both sequence_variants and genetic_markers.
    if (($term->name == 'sequence_variant') OR ($term->name == 'genetic_marker')) {

      // Sequence with Variants Field.
      //---------------------------------
      $info['local__sequence_with_variants'] =  array(
        'field_name' => 'local__sequence_with_variants',
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Sequence with Variants',
        'description' => 'Provides flanking sequence for a given variant showing all known flanking variants.',
        'required' => FALSE,
        'settings' => array(
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => 'feature_id',
          'base_table' => $bundle->data_table,
          'term_vocabulary' => 'local',
          'term_accession' => 'sequence_with_variants',
          'term_name' => 'sequence_with_variants',
        ),
        'widget' => array(
          'type' => 'local__sequence_with_variants_widget',
          'settings' => array(
            'display_label' => 1,
          ),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'local__sequence_with_variants_formatter',
            'settings' => array(),
          ),
        ),
      );

      // Genotype Matrix Link.
      //---------------------------------
      $info['local__genotype_matrix_link'] =  array(
        'field_name' => 'local__genotype_matrix_link',
        'entity_type' => $entity_type,
        'bundle' => $bundle->name,
        'label' => 'Genotype Matrix Link',
        'description' => 'Provides a link to the genotype matrix.',
        'required' => FALSE,
        'settings' => array(
          'auto_attach' => FALSE,
          'chado_table' => $bundle->data_table,
          'chado_column' => $bundle->data_table . '_id',
          'base_table' => $bundle->data_table,
          'term_vocabulary' => 'local',
          'term_accession' => 'genotype_matrix_link',
          'term_name' => 'genotype_matrix_link',
        ),
        'widget' => array(
          'type' => 'local__genotype_matrix_link_widget',
          'settings' => array(
            'display_label' => 1,
          ),
        ),
        'display' => array(
          'default' => array(
            'label' => 'hidden',
            'type' => 'local__genotype_matrix_link_formatter',
            'settings' => array(),
          ),
        ),
      );
    }
  }

  return $info;
}
