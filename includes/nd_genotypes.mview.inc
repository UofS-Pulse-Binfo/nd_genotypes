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
        FROM {feature}
        WHERE feature_id IN (SELECT variant_id FROM {genotype_mview})
        ORDER BY type_id
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT type_id
            FROM {feature}
            WHERE type_id > t.type_id AND feature_id IN (SELECT variant_id FROM {genotype_mview})
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

  //nd_genotypes_seq_backbones
  $resource = chado_query("
    WITH RECURSIVE t AS (
      (SELECT srcfeature_name
        FROM {genotype_mview}
        ORDER BY srcfeature_name
        LIMIT 1)
      UNION ALL
      SELECT
          (SELECT srcfeature_name
            FROM {genotype_mview}
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
 * Method 1: Nd Experiment Tables.
 */
function nd_genotypes_update_mview_for_nd_exp($job_id) {

  $table_name = 'genotype_mview';

  // Create the table if it doesn't already exist.
  chado_create_custom_table(
    $table_name,
    nd_genotypes_genotype_mview_schema_template()
  );

  chado_query("TRUNCATE {".$table_name."}");

  $query = "SELECT
      v.feature_id as variant_id,
      v.name as variant_name,
      loc.srcfeature_id,
      b.name as srcfeature_name,
      loc.fmin,
      loc.fmax,
      s.stock_id as stock_id,
      s.name as stock_name,
      germ.stock_id as germplasm_id,
      germ.name as germplasm_name,
      array_agg(g.description) as alleles,
      mode() WITHIN GROUP (ORDER BY g.description) as consensus_allele
    FROM {nd_experiment} nde
    LEFT JOIN {nd_experiment_project} ndep ON ndep.nd_experiment_id=nde.nd_experiment_id
    LEFT JOIN {nd_geolocation} ndg ON ndg.nd_geolocation_id=nde.nd_geolocation_id
    LEFT JOIN {project} p ON p.project_id=ndep.project_id
    LEFT JOIN {nd_experiment_stock} ndes ON ndes.nd_experiment_id=nde.nd_experiment_id
    LEFT JOIN {stock} s ON s.stock_id=ndes.stock_id
    LEFT JOIN {stock_relationship} sr ON sr.subject_id=s.stock_id
    LEFT JOIN {stock} germ ON germ.stock_id=sr.object_id
    LEFT JOIN {nd_experiment_genotype} ndeg ON ndeg.nd_experiment_id=nde.nd_experiment_id
    LEFT JOIN {genotype} g ON g.genotype_id=ndeg.genotype_id
    LEFT JOIN {feature_genotype} fg ON fg.genotype_id=g.genotype_id
    LEFT JOIN {feature} m ON m.feature_id=fg.feature_id
    LEFT JOIN {feature_relationship} fr ON m.feature_id=fr.subject_id
    LEFT JOIN {feature v} ON v.feature_id=fr.object_id
    LEFT JOIN (
        SELECT *
        FROM ( SELECT *, row_number() OVER (partition by feature_id order by locgroup desc, rank asc) as row_num FROM {featureloc} ) as ordered_locs
        WHERE ordered_locs.row_num = 1
      ) as loc ON loc.feature_id=v.feature_id
    LEFT JOIN {feature} b ON b.feature_id=loc.srcfeature_id
    WHERE sr.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='is_extracted_from')
      AND fr.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='is_marker_of')
    GROUP BY v.feature_id, v.name, s.stock_id, s.name, germ.stock_id, germ.name, loc.srcfeature_id, b.name, loc.fmin, loc.fmax
    ORDER BY b.name ASC, loc.fmin ASC";

    chado_query("
      INSERT INTO {$table_name}
        (variant_id, variant_name, srcfeature_id, srcfeature_name, fmin, fmax, stock_id, stock_name, germplasm_id, germplasm_name, alleles, consensus_allele)
      " . $query);

  // Then report how many were inserted.
  $count = chado_query("SELECT count(*) FROM {".$table_name."}")->fetchField();
  drupal_set_message("Sucessfully updated Genotype materialized view with $count records");

  nd_genotypes_update_mview_cache();
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

  // Create the mview if it doesn't already exist.
  chado_create_custom_table(
    $table_name,
    nd_genotypes_genotype_mview_schema_template()
  );

  // Create the custom genotype_call table if it doesn't exist.
  chado_create_custom_table(
    'genotype_call',
    nd_genotypes_genotype_call_schema_template()
  );

  chado_query("TRUNCATE {".$table_name."}");

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
      array_agg(m.feature_id) as markers,
      array_agg(gc.project_id) as projects,
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
      LEFT JOIN {feature_relationship} fr ON fr.object_id=v.feature_id
      LEFT JOIN {feature} m ON m.feature_id=fr.subject_id
    WHERE
      sr.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='is_extracted_from')
      AND fr.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='is_marker_of')
    GROUP BY gc.variant_id, v.name, loc.srcfeature_id, p.name, loc.fmin, loc.fmax, gc.stock_id, s.name, germ.stock_id, germ.name";


    chado_query("
      INSERT INTO {$table_name}
        (variant_id, variant_name, srcfeature_id, srcfeature_name, fmin, fmax, stock_id, stock_name, germplasm_id, germplasm_name, alleles, markers, projects, consensus_allele)
      " . $query);

  // Then report how many were inserted.
  $count = chado_query("SELECT count(*) FROM {".$table_name."}")->fetchField();
  drupal_set_message("Sucessfully updated Genotype materialized view with $count records");

  nd_genotypes_update_mview_cache();
}

/**
 * Method 3: Genotype Stock Table.
 */
function nd_genotypes_update_mview_for_stock_genotype($job_id) {

  $table_name = 'genotype_mview';

  // Create the table if it doesn't already exist.
  chado_create_custom_table(
    $table_name,
    nd_genotypes_genotype_mview_schema_template()
  );

  chado_query("TRUNCATE {".$table_name."}");

  $query = "SELECT
      v.feature_id as variant_id,
      v.name as variant_name,
      loc.srcfeature_id,
      b.name as srcfeature_name,
      loc.fmin,
      loc.fmax,
      s.stock_id as stock_id,
      s.name as stock_name,
      germ.stock_id as germplasm_id,
      germ.name as germplasm_name,
      array_agg(g.description) as alleles,
      mode() WITHIN GROUP (ORDER BY g.description) as consensus_allele
    FROM {stock_genotype} sg
    LEFT JOIN {stock} s ON s.stock_id=sg.stock_id
    LEFT JOIN {stock_relationship} sr ON s.stock_id=sr.subject_id
    LEFT JOIN {stock} germ ON germ.stock_id=sr.object_id
    LEFT JOIN {genotype} g ON g.genotype_id=sg.genotype_id
    LEFT JOIN {feature_genotype} fg ON fg.genotype_id=g.genotype_id
    LEFT JOIN {feature} m ON m.feature_id=fg.feature_id
    LEFT JOIN {feature_relationship} fr ON m.feature_id=fr.subject_id
    LEFT JOIN {feature} v ON v.feature_id=fr.object_id
    LEFT JOIN (
        SELECT *
        FROM ( SELECT *, row_number() OVER (partition by feature_id order by locgroup desc, rank asc) as row_num FROM {featureloc} ) as ordered_locs
        WHERE ordered_locs.row_num = 1
      ) as loc ON loc.feature_id=v.feature_id
    LEFT JOIN {feature} b ON b.feature_id=loc.srcfeature_id
    WHERE sr.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='is_extracted_from')
      AND fr.type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='is_marker_of')
    GROUP BY v.feature_id, v.name, s.stock_id, s.name, germ.stock_id, germ.name, loc.srcfeature_id, b.name, loc.fmin, loc.fmax";

    chado_query("
      INSERT INTO {$table_name}
        (variant_id, variant_name, srcfeature_id, srcfeature_name, fmin, fmax, stock_id, stock_name, germplasm_id, germplasm_name, alleles, consensus_allele)
      " . $query);

  // Then report how many were inserted.
  $count = chado_query("SELECT count(*) FROM {".$table_name."}")->fetchField();
  drupal_set_message("Sucessfully updated Genotype materialized view with $count records");

  nd_genotypes_update_mview_cache();
}

/**
 * Provides the schema for creating new genotype_mview partitions.
 *
 * Fields:
 *  - variant_id: references the feature describing the loci with variation.
 *  - variant_name: the name of the variant.
 *  - srcfeature_id: references the feature the variant is located on. Ideally this is the chromosome.
 *  - srcfeature_name: the name of the srcfeature; hopefully the name of the chromosome ;-).
 *  - fmin: the start position of the variant on the srcfeature; positions are 0-indexed.
 *  - fmax: the end position of the variant on the srcfeature; posoitions are 0-indexed.
 *  - stock_id: references the stock assayed.
 *  - stock_name: the name of the stock assayed.
 *  - germplasm_id:references the germplasm the DNA stock was extracted from.
 *  - germplasm_name: the name of the germplasm.
 *  - consensus_allele: the consensus allele for this stock.
 *  - alleles: a postgresql array of alleles detected for this stock.
 *  - marker_calls: a JSONB array defining the markers and subsequent calls;
 *     generated using nd_genotypes_encode_markers().
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
      'markers' => array(
        'pgsql_type' => 'int[]',
        'not null' => TRUE,
      ),
      'projects' => array(
        'pgsql_type' => 'int[]',
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
      'variant_idx' => array('variant_id'),
      'loc_idx' => array('srcfeature_name', 'fmin'),
      'stock_idx' => array('stock_id'),
      'germ_idx' => array('germplasm_id'),
    ),
  );
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
      'idx1' => array('variant_id'),
      'idx2' => array('marker_id'),
      'idx3' => array('project_id'),
      'idx4' => array('stock_id'),
    ),
  );
}
