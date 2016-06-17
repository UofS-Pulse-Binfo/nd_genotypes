<?php
/**
 * @file
 * Provides functionality for managing the genotype_experiment materialized view.
 *
 * NOTE: The genotype mview is hard-partitioned by organism then chromosome of the variant.
 */

/**
 * Method 1: Nd Experiment Tables.
 */
function nd_genotypes_update_mview_for_nd_exp($job_id) {

  $table_name = 'genotype_mview';

  // Create the table if it doesn't already exist.
  chado_create_custom_table(
    $table_name,
    nd_genotypes_genotype_mview_schema_template()
  );

}

/**
 * Method 2: Genotype Call Custom Table.
 *
 * Genotype_call custom table:
 *   - genotype_call_id: primary key for this table,
 *   - variant_id: the feature_id of the variant,
 *   - marker_id: the feature_id of the marker,
 *   - genotype_id: the genotype_id of the allele call for the given marker/stock combination
 *   - project_id: the project_id of the over-aching project
 *   - stock_id: the stock_id of the germplasm assayed
 *   - meta_data: JSONB blob to store meta-data
 *
 * Approach:
 *  1) using a single query to populate the entire table
 *  2) iterate over that table
 *      a) refining the consensus call and
 *      b) filling in the marker details.
 * Notice that we will attempt to create the table if it doesn't already exist.
 */
function nd_genotypes_update_mview_for_genotype_call($job_id) {

  $table_name = 'genotype_mview';

  // Create the table if it doesn't already exist.
  chado_create_custom_table(
    $table_name,
    nd_genotypes_genotype_mview_schema_template()
  );

  $query = "SELECT
      gc.variant_id,
      v.name as variant_name,
      loc.srcfeature_id,
      p.name as srcfeature_name,
      loc.fmin,
      loc.fmax,
      gc.stock_id,
      s.name as stock_name,
      germ.stock_id as germplasm_id,
      germ.name as germplasm_name,
      array_agg(g.description) as alleles,
      mode() WITHIN GROUP (ORDER BY g.description) as consensus_allele
    FROM {genotype_call} gc
      LEFT JOIN {feature} v ON v.feature_id=gc.variant_id
      LEFT JOIN (
          SELECT *
          FROM ( SELECT *, row_number() OVER (partition by feature_id order by locgroup desc, rank asc) as row_num FROM {featureloc} ) as ordered_locs
          WHERE ordered_locs.row_num = 1
        ) as loc ON loc.feature_id=v.feature_id
      LEFT JOIN {feature} p ON p.feature_id=loc.srcfeature_id
      LEFT JOIN {stock} s ON s.stock_id=gc.stock_id
      LEFT JOIN {stock_relationship} sr ON sr.subject_id=s.stock_id
      LEFT JOIN {stock} germ ON germ.stock_id=sr.object_id
      LEFT JOIN {genotype} g ON g.genotype_id=gc.genotype_id
    WHERE sr.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='is_extracted_from')
    GROUP BY gc.variant_id, v.name, loc.srcfeature_id, p.name, loc.fmin, loc.fmax, gc.stock_id, s.name, germ.stock_id, germ.name";


    chado_query("
      INSERT INTO {$table_name}
        (variant_id, variant_name, srcfeature_id, srcfeature_name, fmin, fmax, stock_id, stock_name, germplasm_id, germplasm_name, alleles, consensus_allele)
      " . $query);

}

/**
 * Method 3: Genotype Stock Table.
 */
function nd_genotypes_update_mview_for_genotype_stock($job_id) {

  $table_name = 'genotype_mview';

  // Create the table if it doesn't already exist.
  chado_create_custom_table(
    $table_name,
    nd_genotypes_genotype_mview_schema_template()
  );

}

/**
 * Provides the schema for creating new genotype_mview partitions.
 */
function nd_genotypes_genotype_mview_schema_template() {

  return array(
    'description' => 'A materialized view for storing genotype information.',
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
      'consensus_allele' => array(
        'description' => 'The consensus allele of all germplasm for this variant.',
        'type' => 'varchar',
        'length' => 255,
        'not null' => TRUE,
      ),
      'alleles' => array(
        'pgsql_type' => 'text[]',
        'not null' => TRUE,
      ),
      'marker_calls' => array(
        'pgsql_type' => 'jsonb',
      ),
      'genotype_mview_id' => array(
        'type' => 'serial',
      ),
    ),
    'primary key' => array('genotype_mview_id'),
    'unique keys' => array(),
    'foreign keys' => array(
      'feature' => array(
        'table' => 'feature',
        'columns' => array(
          'variant_id' => 'feature_id',
          'srcfeature_id' => 'feature_id'
        ),
      ),
      'stock' => array(
        'table' => 'stock',
        'columns' => array(
          'stock_id' => 'stock_id',
          'germplasm_id' => 'stock_id'
        ),
      ),
    ),
    'indexes' => array(
      'idx1' => array('variant_id'),
      'idx2' => array('srcfeature_id'),
      'idx3' => array('stock_id'),
      'idx4' => array('germplasm_id'),
    ),
  );
}
