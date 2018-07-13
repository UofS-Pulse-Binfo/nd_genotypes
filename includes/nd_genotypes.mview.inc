<?php
/**
 * @file
 * Provides functionality for managing the genotype_experiment materialized view.
 *
 * NOTE: The genotype mview is hard-partitioned by organism then chromosome of the variant.
 */

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

  $version = variable_get('pgsql_version', 9.2);

  $schema = array(
    'description' => 'A listing of all the variants sorted by location.',
    'fields' => array(
      'variant_id' => array(
        'name' => 'variant_id',
        'title' => 'Variant ID',
        'description' => 'Links to the feature describing the loci with variation.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'variant_name' => array(
        'name' => 'variant_name',
        'title' => 'Variant Name',
        'description' => 'The name of the variant.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'variant_type' => array(
        'name' => 'variant_type',
        'title' => 'Variant Type',
        'description' => 'The type of variant (e.g. SNP, MNP, Indel).',
        'type' => 'varchar',
        'length' => 255,
      ),
      'srcfeature_id' => array(
        'name' => 'srcfeature_id',
        'title' => 'Parent Feature ID',
        'description' => 'Links to the feature the variant is located on. Ideally this is the chromosome.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
      ),
      'srcfeature_name' => array(
        'name' => 'srcfeature_name',
        'title' => 'Parent Feature Name',
        'description' => 'The name of the srcfeature; hopefully the name of the chromosome.',
        'type' => 'varchar',
        'length' => 255,
      ),
      'fmin' => array(
        'name' => 'fmin',
        'title' => 'Start Position',
        'descrption' => 'Start position on the scrfeature as described by the featureloc table.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
      ),
      'fmax' => array(
        'name' => 'fmax',
        'title' => 'End Position',
        'descrption' => 'End position on the scrfeature as described by the featureloc table.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
      ),
      'meta_data' => array(
        'name' => 'meta_data',
        'title' => 'Meta data',
        'description' => 'JSON storage of any addition meta-data for the call.',
        'pgsql_type' => ($version >= 9.4) ? 'jsonb' : 'json',
        'views_type' => 'text',
      ),
      'ndg_variants_id' => array(
        'name' => 'ndg_variants_id',
        'title' => 'NDG Variant ID',
        'description' => 'The primary key',
        'pgsql_type' => 'bigserial',
        'views_type' => 'int',
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

  chado_create_custom_table( $tablename, $schema, TRUE, NULL, FALSE );

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
    // CREATE INDEX ndgvariants_variant_id ON mview_ndg_variants (variant_id);
    // Used for variant/marker pages when selecting allele calls.
    'variant_id' => array(
      'columns' => array('variant_id')
    ),
    // CREATE INDEX ndgvariants_variant_name ON mview_ndg_variants (variant_name);
    // Used for genotype matrix when filtering to a list of variants.
    'variant_name' => array(
      'columns' => array('variant_name')
    ),
  );
  foreach ($indexes as $name_suffix => $details) {
    $index_name = str_replace('_','',$tablename) . '_' . $name_suffix;
    chado_query("CREATE INDEX $index_name ON {".$tablename."} (" . implode(', ', $details['columns']) . ')');
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
 *  - meta_data: a json blon storing any quality information
 */
function nd_genotypes_create_mview_ndg_calls($tablename = 'mview_ndg_calls') {

  $version = variable_get('pgsql_version', 9.2);

  $schema = array(
    'description' => 'A listing of all the marker calls.',
    'fields' => array(
      'variant_id' => array(
        'name' => 'variant_id',
        'title' => 'Variant ID',
        'description' => 'The feature_id of the loci with variation.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'marker_id' => array(
        'name' => 'marker_id',
        'title' => 'Marker ID',
        'description' => 'The feature_id of the marker.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'marker_name' => array(
        'name' => 'marker_name',
        'title' => 'Marker Name',
        'description' => 'The name of the marker.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'marker_type' => array(
        'name' => 'marker_type',
        'title' => 'Marker Type',
        'description' => 'The type of marker.',
        'type' => 'varchar',
        'length' => 255,
      ),
      'stock_id' => array(
        'name' => 'stock_id',
        'title' => 'Stock ID',
        'description' => 'Links to the DNA stock assayed by the marker.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'stock_name' => array(
        'name' => 'stock_name',
        'title' => 'Stock Name',
        'description' => 'The name of the DNA stock assayed.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'germplasm_id' => array(
        'name' => 'germplasm_id',
        'title' => 'Germplasm ID',
        'description' => 'Links to the parent germplasm the DNA was extracted from.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'germplasm_name' => array(
        'name' => 'germplasm_name',
        'title' => 'Germplasm Name',
        'description' => 'The name of the parent germplasm.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'project_id' => array(
        'name' => 'project_id',
        'title' => 'Project ID',
        'description' => 'The project_id of the project the marker was assayed as part of.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'genotype_id' => array(
        'name' => 'genotype_id',
        'title' => 'Genotype ID',
        'description' => 'The genotype_id of the resultant allele call.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'allele_call' => array(
        'name' => 'allele_call',
        'title' => 'Allele Call',
        'description' => 'the result of the marker assay (ie: AA).',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'meta_data' => array(
        'name' => 'meta_data',
        'title' => 'Meta data',
        'description' => 'JSON storage of any addition meta-data for the call.',
        'pgsql_type' => ($version >= 9.4) ? 'jsonb' : 'json',
        'views_type' => 'text',
      ),
      'ndg_call_id' => array(
        'name' => 'ndg_call_id',
        'title' => 'NDG Call ID',
        'description' => 'the primary key',
        'pgsql_type' => 'bigserial',
        'views_type' => 'int',
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

  chado_create_custom_table($tablename, $schema, TRUE, NULL, FALSE);

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
    // CREATE INDEX ndgcalls_project_id ON mview_ndg_calls (project_id);
    // Used for the genotype matrix when restricting to projects.
    'germplasm_id' => array(
      'columns' => array('germplasm_id')
    ),
  );
  foreach ($indexes as $name_suffix => $details) {
    $index_name = str_replace('_','',$tablename) . '_' . $name_suffix;
    chado_query("CREATE INDEX $index_name ON {".$tablename."} (" . implode(', ', $details['columns']) . ')');
  }

}

/**
 * A listing of all the germplasm genotyped per partition.
 *
 * @param $tablename
 *   The name of the table. At this point there is no reason to change this as partitions
 *   are handled within the same table.
 *
 * Columns:
 *  - germplasm_id: the stock_id of the germplasm genotyped.
 *  - germplasm_name: the stock.name
 *  - organism_id: the foreign key to the organism of this germplasm
 *  - organism_genus: the genus of this germplasm
 *  - partition: the partition this germplasm's genotypic calls are recorded in
 */
function nd_genotypes_create_mview_ndg_germplasm_genotyped($table_name = 'mview_ndg_germplasm_genotyped') {

  $schema = array(
    'table' => $table_name,
    'description' => 'A more compact way to store genotype calls.',
    'fields' => array(
      'germplasm_id' => array(
        'name' => 'germplasm_id',
        'title' => 'Germplasm ID',
        'description' => 'Links to the germplasm stock.',
        'type' => 'int',
        'not null' => TRUE,
      ),
      'germplasm_name' => array(
        'name' => 'germplasm_name',
        'title' => 'Germplasm Name',
        'description' => 'The name of the germplasm.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'organism_id' => array(
        'name' => 'organism_id',
        'title' => 'Organism ID',
        'description' => 'The organism the stock is from.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'organism_genus' => array(
        'name' => 'organism_genus',
        'title' => 'Organism Genus',
        'description' => 'The genus of the organism the stock is from.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'partition' => array(
        'name' => 'partition',
        'title' => 'Partition',
        'description' => 'The partition the germplasm has calls in.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array('germplasm_id'),
    'unique keys' => array(),
    'foreign keys' => array(
      'stock' => array(
        'table' => 'stock',
        'columns' => array(
          'germplasm_id' => 'stock_id'
        ),
      ),
    ),
    'indexes' => array(
      'ndg_germgenotyped_name' => array('germplasm_name'),
      'ndg_germgenotyped_genus' => array('organism_genus'),
      'ndg_germgenotyped_partition' => array('partition'),
    ),
  );

  chado_create_custom_table( $table_name, $schema, TRUE, NULL, FALSE );

}

/**
 * A table listing summary statistics for genotypes handled by this module.
 *
 * @param $tablename
 *   The name of the table. At this point there is no reason to change this as partitions
 *   are handled within the same table.
 *
 * Columns:
 *  - stat_name: The human-readable name of the summary statistic recorded in this row (e.g. Number of Genotypic Calls).
 *  - stat_id: The machine name of the summary statistic recorded in this row (e.g. num_calls).
 *  - stat_unit: The unit of the statistic (e.g. count).
 *  - partition: The partition this summary statistic applies to.
 *  - value: The value of the statistic.
 */
function nd_genotypes_create_mview_summary($table_name = 'mview_ndg_summary') {

  $schema = array(
    'table' => $table_name,
    'description' => 'A summary of the genotypes in the various mviews.',
    'fields' => array(
      'stat_name' => array(
        'name' => 'stat_name',
        'title' => 'Statistic Name',
        'description' => 'The human-readable name of the summary statistic recorded in this row (e.g. Number of Genotypic Calls).',
        'type' => 'text',
        'not null' => TRUE,
      ),
      'stat_id' => array(
        'name' => 'stat_id',
        'title' => 'Statistic ID',
        'description' => 'The machine name of the summary statistic recorded in this row (e.g. num_calls).',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'stat_unit' => array(
        'name' => 'stat_unit',
        'title' => 'Statistic Unit',
        'description' => 'The unit of the statistic (e.g. count).',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'partition' => array(
        'name' => 'partition',
        'title' => 'Parition',
        'description' => 'The partition this summary statistic applies to.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'value' => array(
        'name' => 'value',
        'title' => 'Value',
        'description' => 'The value of the statistic.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      )
    ),
    'primary key' => array('stat_id', 'partition'),
    'unique keys' => array(),
    'foreign keys' => array(),
    'indexes' => array(),
  );

  chado_create_custom_table( $table_name, $schema, TRUE, NULL, FALSE );

}

/**
 * The schema for the custom genotype_call table.
 */
function nd_genotypes_genotype_call_schema_template() {

  $version = variable_get('pgsql_version', 9.2);

  return array(
    'description' => 'A more compact way to store genotype calls.',
    'fields' => array(
      'genotype_call_id' => array(
        'name' => 'genotype_call_id',
        'title' => 'Call ID',
        'description' => 'The primary key',
        'pgsql_type' => 'bigserial',
        'views_type' => 'int',
      ),
      'variant_id' => array(
        'name' => 'variant_id',
        'title' => 'Variant ID',
        'description' => 'Links to the feature describing the loci with variation.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'marker_id' => array(
        'name' => 'marker_id',
        'title' => 'Marker ID',
        'description' => 'Links to the feature describing the marker.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'genotype_id' => array(
        'name' => 'genotype_id',
        'title' => 'Genotype ID',
        'description' => 'Links to the allele call.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'project_id' => array(
        'name' => 'project_id',
        'title' => 'Project ID',
        'description' => 'Links to the project grouping calls from a single analysis.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'stock_id' => array(
        'name' => 'stock_id',
        'title' => 'Stock ID',
        'description' => 'Links to the DNA stock assayed by the marker.',
        'pgsql_type' => 'bigint',
        'views_type' => 'int',
        'not null' => TRUE,
      ),
      'meta_data' => array(
        'name' => 'meta_data',
        'title' => 'Meta data',
        'description' => 'JSON storage of any addition meta-data for the call.',
        'pgsql_type' => ($version >= 9.4) ? 'jsonb' : 'json',
        'views_type' => 'text',
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
