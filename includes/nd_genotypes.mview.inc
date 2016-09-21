<?php
/**
 * @file
 * Provides functionality for managing the genotype_experiment materialized view.
 *
 * NOTE: The genotype mview is hard-partitioned by organism then chromosome of the variant.
 */

/**
 * Update cached information after the materialized view has been updated.
 */
function nd_genotypes_update_mview_cache() {

  // The types of variants (e.g. SNP, MNP, Indel) with the key being the type_id.
  $resource = chado_query('
    WITH RECURSIVE t AS (
      (SELECT type_id
        FROM {feature} f
        LEFT JOIN {mview_ndg_variants} v ON v.variant_id=f.feature_id
        ORDER BY type_id
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT type_id
            FROM {feature} f
            LEFT JOIN {mview_ndg_variants} v ON v.variant_id=f.feature_id
            WHERE type_id > t.type_id
            ORDER BY type_id
            LIMIT 1)
        FROM t
        WHERE t.type_id IS NOT NULL
    )
    SELECT t.type_id, cvt.name as type_name FROM t LEFT JOIN {cvterm} cvt ON cvt.cvterm_id=t.type_id WHERE t.type_id IS NOT NULL');
  $types = array();
  foreach ($resource as $r) {
    $types[$r->type_id] = $r->type_name;
  }
  variable_set('nd_genotypes_variant_types', serialize($types));

  // The types of markers (e.g. GBS, Exome Capture) as stored in the featureprop table.
  $marker_type = chado_query("SELECT cvterm_id FROM {cvterm} WHERE name='marker_type'")->fetchField();
  $resource = chado_query("
    WITH RECURSIVE t AS (
      (SELECT value
        FROM chado.featureprop
        ORDER BY value
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT value
            FROM chado.featureprop
            WHERE value > t.value
            ORDER BY value
            LIMIT 1)
        FROM t
        WHERE t.value IS NOT NULL
    )
    SELECT t.value FROM t WHERE (SELECT type_id FROM chado.featureprop WHERE featureprop.value=t.value LIMIT 1)=:marker_type AND t.value IS NOT NULL",
    array(':marker_type' => $marker_type));
  $types = array();
  foreach ($resource as $r) {
    $types[$r->value] = $r->value;
  }
  variable_set('nd_genotypes_marker_types', serialize($types));

  // The backbone names (ie: Chr1, Chr2, etc.).
  $resource = chado_query("
    WITH RECURSIVE t AS (
      (SELECT srcfeature_name
        FROM {mview_ndg_variants}
        ORDER BY srcfeature_name
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT srcfeature_name
            FROM {mview_ndg_variants}
            WHERE srcfeature_name > t.srcfeature_name
            ORDER BY srcfeature_name
            LIMIT 1)
        FROM t
        WHERE t.srcfeature_name IS NOT NULL
    )
    SELECT t.srcfeature_name FROM t WHERE t.srcfeature_name IS NOT NULL");
  $b = array();
  foreach ($resource as $r) {
    $b[$r->srcfeature_name] = $r->srcfeature_name;
  }
  variable_set('nd_genotypes_seq_backbones', serialize($b));

}

/**
 * A listing of all the variants sorted by location. If a variant is in more than
 * one location then it will be repeated in the listing.
 *
 * The sync functions ensure that only locations from the specified genome
 * assembly are present.
 *
 * @param $tablename
 *   The name of the table. This allows you to create hard-partitions for this mview.
 *
 * Columns:
 *  - ndg_variants_id: primary key
 *  - variant_id: the feature_id of the variant
 *  - variant_name: the feature.name of the variant
 *  - srcfeature_id: the feature_id of the backbone the variant is located on
 *  - srcfeature_name: the feature.name of the backbone the variant is located on
 *  - fmin: the start location on the backbone
 *  - fmax: the end position on the backbone
 */
function nd_genotypes_create_mview_ndg_variants($tablename = 'mview_ndg_variants') {

  $schema = array(
    'description' => 'A listing of all the variants sorted by location.',
    'fields' => array(
      'variant_id' => array(
        'description' => 'Links to the feature describing the loci with variation.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'variant_name' => array(
        'description' => 'The name of the variant.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'srcfeature_id' => array(
        'description' => 'Links to the feature the variant is located on. Ideally this is the chromosome.',
        'type' => 'int',
      ),
      'srcfeature_name' => array(
        'description' => 'The name of the srcfeature; hopefully the name of the chromosome.',
        'type' => 'varchar',
        'length' => 255,
      ),
      'fmin' => array(
        'descrption' => 'Start position on the scrfeature as described by the featureloc table.',
        'type' => 'int',
      ),
      'fmax' => array(
        'descrption' => 'End position on the scrfeature as described by the featureloc table.',
        'type' => 'int',
      ),
      'ndg_variants_id' => array(
        'type' => 'serial',
      ),
    ),
    'primary key' => array('ndg_variants_id'),
    'unique keys' => array(),
    'foreign keys' => array(
      'feature' => array(
        'table' => 'feature',
        'columns' => array(
          'variant_id' => 'feature_id',
          'srcfeature_id' => 'feature_id'
        ),
      ),
    ),
    'indexes' => array(    ),
  );

  chado_create_custom_table( $tablename, $schema );
}

/**
 * Create Indexes for the mview_ndg_variants table.
 *
 * Note: This is done seperately since it is inefficient to create indexes before bulk
 * loading the data. Thus we create the table with no indexes or drop the indexes if the
 * table is pre-existing, load the data and then use this function to add the indexes
 * after the fact.
 *
 * @param $tablename
 *   The name of the table. This allows you to create hard-partitions for this mview.
 */
function nd_genotypes_create_mview_ndg_variants_indexes($tablename = 'mview_ndg_variants') {

  $indexes = array(
    // CREATE INDEX ndg_variants_loc ON mview_ndg_variants (srcfeature_name ASC, fmin ASC);
    // Used for paging the genotype matrix as well as restricting to a range.
    'loc' => array(
      'columns' => array('srcfeature_name ASC', 'fmin ASC')
    ),
  );
  foreach ($indexes as $name_suffix => $details) {
    $index_name = str_replace('_','',$tablename) . '_' . $name_suffix;
    chado_query("CREATE INDEX $index_name ON $tablename (" . implode(', ', $details['columns']) . ')');
  }

}

/**
 * A listing of all the marker calls.
 *
 * @param $tablename
 *   The name of the table. This allows you to create hard-partitions for this mview.
 *
 * Columns:
 *  - ndg_call_id: primary key
 *  - variant_id: the feature_id of the variant
 *  - marker_id: the feature_id of the marker
 *  - marker_name: the feature.name of the marker
 *  - marker_type: the type of the marker (featureprop of type=marker_type)
 *  - stock_id: the stock_id of the DNA extraction
 *  - stock_name: the stock.name of the DNA extraction
 *  - germplasm_id: the stock_id of the germplasm the stock was extracted from
 *  - germplasm_name: the stock.name of the germplasm the stock was extracted from
 *  - project_id: the project_id of the project this call was generated as part of
 *  - genotype_id: the genotype_id of the resultant allele call
 *  - allele_call: the result of the marker assay (ie: AA)
 *  - marker_data: a json blon storing any quality information
 */
function nd_genotypes_create_mview_ndg_calls($tablename = 'mview_ndg_calls') {

  $schema = array(
    'description' => 'A listing of all the marker calls.',
    'fields' => array(
      'variant_id' => array(
        'description' => 'The feature_id of the loci with variation.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'marker_id' => array(
        'description' => 'The feature_id of the marker.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'marker_name' => array(
        'description' => 'The name of the marker.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'marker_type' => array(
        'description' => 'The type of marker.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'stock_id' => array(
        'description' => 'Links to the DNA stock assayed by the marker.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'stock_name' => array(
        'description' => 'The name of the DNA stock assayed.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'germplasm_id' => array(
        'description' => 'Links to the parent germplasm the DNA was extracted from.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'germplasm_name' => array(
        'description' => 'The name of the parent germplasm.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'project_id' => array(
        'description' => 'The project_id of the project the marker was assayed as part of.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'genotype_id' => array(
        'description' => 'The genotype_id of the resultant allele call.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'allele_call' => array(
        'description' => 'the result of the marker assay (ie: AA).',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'marker_data' => array(
        'pgsql_type' => 'jsonb',
      ),
      'ndg_call_id' => array(
        'type' => 'serial',
      ),
    ),
    'primary key' => array('ndg_call_id'),
    'unique keys' => array(),
    'foreign keys' => array(
      'feature' => array(
        'table' => 'feature',
        'columns' => array(
          'variant_id' => 'feature_id',
          'marker_id' => 'feature_id'
        ),
      ),
      'stock' => array(
        'table' => 'stock',
        'columns' => array(
          'stock_id' => 'stock_id',
          'germplasm_id' => 'stock_id'
        ),
      ),
      'project' => array(
        'table' => 'stock',
        'columns' => array(
          'project_id' => 'project_id',
        ),
      ),
      'genotype' => array(
        'table' => 'genotype',
        'columns' => array(
          'genotype_id' => 'genotype_id',
        ),
      ),
    ),
    // We do not add indexes when creating the table because it slows down bulk
    // populating of the table. Indexes will be added after the table is sync'd
    'indexes' => array(),
  );

  chado_create_custom_table( $tablename, $schema );

}

/**
 * Create Indexes for the mview_ndg_calls table.
 *
 * Note: This is done seperately since it is inefficient to create indexes before bulk
 * loading the data. Thus we create the table with no indexes or drop the indexes if the
 * table is pre-existing, load the data and then use this function to add the indexes
 * after the fact.
 *
 * @param $tablename
 *   The name of the table. This allows you to create hard-partitions for this mview.
 */
function nd_genotypes_create_mview_ndg_calls_indexes($tablename = 'mview_ndg_calls') {

  $indexes = array(
    // CREATE INDEX ndgcalls_variant_id ON mview_ndg_calls (variant_id);
    // Used for updating the mview_ndg_variants mview to chunk the records.
    'variant_id' => array(
      'columns' => array('variant_id')
    ),
    // CREATE INDEX ndgcalls_germplasm_id ON mview_ndg_calls (germplasm_id);
    // Used for the genotype matrix when restricting to germplasm to add.
    'germplasm_id' => array(
      'columns' => array('germplasm_id')
    ),
  );
  foreach ($indexes as $name_suffix => $details) {
    $index_name = str_replace('_','',$tablename) . '_' . $name_suffix;
    chado_query("CREATE INDEX $index_name ON $tablename (" . implode(', ', $details['columns']) . ')');
  }

}

/**
 * The schema for the custom genotype_call table.
 */
function nd_genotypes_genotype_call_schema_template() {

  return array(
    'description' => 'A more compact way to store genotype calls.',
    'fields' => array(
      'genotype_call_id' => array(
        'type' => 'serial',
      ),
      'variant_id' => array(
        'description' => 'Links to the feature describing the loci with variation.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'marker_id' => array(
        'description' => 'Links to the feature describing the marker.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'genotype_id' => array(
        'description' => 'Links to the allele call.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'project_id' => array(
        'description' => 'Links to the project grouping calls from a single analysis.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'stock_id' => array(
        'description' => 'Links to the DNA stock assayed by the marker.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'marker_data' => array(
        'pgsql_type' => 'jsonb',
      ),
    ),
    'primary key' => array('genotype_call_id'),
    'unique keys' => array(),
    'foreign keys' => array(
      'feature' => array(
        'table' => 'feature',
        'columns' => array(
          'variant_id' => 'feature_id',
          'marker_id' => 'feature_id'
        ),
      ),
      'genotype' => array(
        'table' => 'genotype',
        'columns' => array(
          'genotype_id' => 'genotype_id'
        ),
      ),
      'project' => array(
        'table' => 'project',
        'columns' => array(
          'project_id' => 'project_id'
        ),
      ),
      'stock' => array(
        'table' => 'stock',
        'columns' => array(
          'stock_id' => 'stock_id'
        ),
      ),
    ),
    'indexes' => array(
      'genotypecall_variant_id' => array('variant_id'),
      'genotypecall_marker_id' => array('marker_id'),
      'genotypecall_project_id' => array('project_id'),
      'genotypecall_stock_id' => array('stock_id'),
    ),
  );
}
