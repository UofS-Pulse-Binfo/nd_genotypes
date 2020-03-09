<?php

namespace Tests\DatabaseSeeders;

use StatonLab\TripalTestSuite\Database\Seeder;
use Faker\Factory;

class GeneticMarkerSeeder extends Seeder
{
    public $variant_id;
    public $chromosome_id;
    public $organism_id;
    public $position;
    public $inital_data;
    public $markers;
    public $genotypes;

    /**
     * Seeds the database with users.
     *
     * @return void
     */
    public function up($organism_id = NULL, $chr_id = NULL) {
      $faker = Factory::create();

      // SQL for retrieving the cvterm (assumes sequence cv).
      $get_type_sql = "SELECT cvterm_id FROM {cvterm} WHERE name=:type and cv_id IN (SELECT cv_id FROM {cv} WHERE name='sequence')";

      // Generate a position for this marker.
      $position = rand(100,600);
      $this->position;

      // Organism.
      if ($organism_id === NULL) {
        $organism = factory('chado.organism')->create();
        $organism_id = $organism->organism_id;
        $this->organism_id = $organism_id;
      }
      else {
        $this->organism_id = $organism_id;
        $organism = chado_select_record('organism', ['*'], ['organism_id' => $organism_id]);
        $organism = array_pop($organism);
      }

      // Chromosome.
      if ($chr_id === NULL) {
        $chr_type_id = chado_query($get_type_sql,
          [':type' => 'chromosome'])->fetchField();
        $chr = factory('chado.feature')->create([
          'organism_id' => $organism->organism_id,
          'type_id' => $chr_type_id,
        ]);
        $chr_id = $chr->feature_id;
        $this->chromosome_id = $chr_id;
      }
      else {
        $this->chromosome_id = $chr_id;
        $chr = chado_select_record('feature', ['*'], ['feature_id' => $chr_id]);
        $chr = array_pop($chr);
      }

      // Variant.
      $variant_type_id = chado_query($get_type_sql,
        [':type' => 'sequence_variant'])->fetchField();
      $variant = factory('chado.feature')->create([
        'organism_id' => $organism->organism_id,
        'type_id' => $variant_type_id,
      ]);
      $variant_id = $variant->feature_id;
      $this->variant_id = $variant_id;

      // -- with position on chromosome.
      $pos_variant = chado_insert_record('featureloc', [
        'srcfeature_id' => $chr_id,
        'feature_id' => $variant_id,
        'fmin' => $position,
        'fmax' => $position+1,
      ]);

      // -- and a variant type (e.g. SNP).
      $variant_type_id = nd_genotypes_get_type_id('Variant Type');
      $variant_prop = chado_insert_record('featureprop', [
        'feature_id' => $variant_id,
        'type_id' => $variant_type_id,
        'value' => 'SNP',
      ]);

      // Genetic Marker.
      $marker_type_id = chado_query($get_type_sql,
        [':type' => 'genetic_marker'])->fetchField();
      $marker = factory('chado.feature')->create([
        'organism_id' => $organism->organism_id,
        'type_id' => $marker_type_id,
      ]);
      $marker_id = $marker->feature_id;

      // -- with position on chromosome.
      $pos_marker = chado_insert_record('featureloc', [
        'srcfeature_id' => $chr_id,
        'feature_id' => $marker_id,
        'fmin' => $position,
        'fmax' => $position+1,
      ]);

      // -- and a marker type (e.g. Exome Capture).
      $marker_type_id = nd_genotypes_get_type_id('Marker Type');
      $marker_prop = chado_insert_record('featureprop', [
        'feature_id' => $marker_id,
        'type_id' => $marker_type_id,
        'value' => $faker->words(3, TRUE),
      ]);

      // -- relationship with variant.
      $rel_type = chado_query($get_type_sql,
        [':type' => 'variant_of'])->fetchField();
      variable_set('nd_genotypes_rel_type_id', $rel_type);
      $variant_position = 'subject';
      variable_set('nd_genotypes_rel_position', $variant_position);
      if ($variant_position == 'subject') {
        $rel = chado_insert_record('feature_relationship', [
          'subject_id' => $variant_id,
          'type_id' => $rel_type,
          'object_id' => $marker_id,
        ]);
      }
      else {
        $rel = chado_insert_record('feature_relationship', [
          'subject_id' => $marker_id,
          'type_id' => $rel_type,
          'object_id' => $variant_id,
        ]);
      }

      $data = [
        'organism' => (array) $organism,
        'chromosome' => [
          'chromosome_id' => $chr_id,
          'record' => $chr,
        ],
        'variant' => [
          'variant_id' => $variant_id,
          'record' => $variant
        ],
        'variant_type' => [
          'record' => $variant_prop,
          'type' => $variant_prop['value'],
        ],
        'marker' => [
          'marker_id' => $marker_id,
          'record' => $marker,
        ],
        'marker_type' => [
          'record' => $marker_prop,
          'type' => $marker_prop['value'],
        ],
        'relationship' => [
          'type_id' => $rel_type,
          'variant_position' => $variant_position,
          'record' => $rel,
        ],
        'position' => [
          'start' => $position,
          'stop' => $position+1,
          'records' => [
            'marker' => $pos_marker,
            'variant' => $pos_variant,
          ],
        ],
      ];
      $this->inital_data = $data;
      $marker->marker_type = $marker_prop['value'];
      $this->markers = [$marker];

      return $data;
    }

    /**
     * Add a second marker to the same variant.
     */
    public function addMarker() {
      $faker = Factory::create();

      // SQL for retrieving the cvterm (assumes sequence cv).
      $get_type_sql = "SELECT cvterm_id FROM {cvterm} WHERE name=:type and cv_id IN (SELECT cv_id FROM {cv} WHERE name='sequence')";

      // Genetic Marker.
      $marker_type_id = chado_query($get_type_sql,
        [':type' => 'genetic_marker'])->fetchField();
      $marker = factory('chado.feature')->create([
        'organism_id' => $this->organism_id,
        'type_id' => $marker_type_id,
      ]);
      $marker_id = $marker->feature_id;

      // -- with position on chromosome.
      $pos_marker = chado_insert_record('featureloc', [
        'srcfeature_id' => $this->chromosome_id,
        'feature_id' => $marker_id,
        'fmin' => $this->position,
        'fmax' => $this->position+1,
      ]);

      // -- and a marker type (e.g. Exome Capture).
      $marker_type_id = nd_genotypes_get_type_id('Marker Type');
      $marker_prop = chado_insert_record('featureprop', [
        'feature_id' => $marker_id,
        'type_id' => $marker_type_id,
        'value' => $faker->words(3, TRUE),
      ]);

      // -- relationship with variant.
      $rel_type = variable_get('nd_genotypes_rel_type_id', 0);
      $variant_position = variable_get('nd_genotypes_rel_position', 'subject');
      if ($variant_position == 'subject') {
        $rel = chado_insert_record('feature_relationship', [
          'subject_id' => $this->variant_id,
          'type_id' => $rel_type,
          'object_id' => $marker_id,
        ]);
      }
      else {
        $rel = chado_insert_record('feature_relationship', [
          'subject_id' => $marker_id,
          'type_id' => $rel_type,
          'object_id' => $this->variant_id,
        ]);
      }

      $marker->marker_type = $marker_prop['value'];
      $this->markers[] = $marker;
      return [
        'marker_id' => $marker_id,
        'record' => $marker,
        'type' => $marker_prop['value'],
      ];

    }

    /**
     * Add Genotypes to existing markers.
     */
    public function addGenotypes($max = 1, $stocks = []) {

      // Create a project.
      $project = factory('chado.project')->create();

      // Insert all the alleles.
      $alleles = ['AA', 'TT', 'CC', 'GG', 'AT', 'AC', 'AG', 'TC', 'TG', 'CG'];
      foreach ($alleles as $call) {
        $genotype = chado_select_record('genotype', ['*'], [
          'uniquename' => $call,
          'description' => $call,
          'type_id' => ['name' => 'SNP', 'cv_id' => ['name' => 'sequence']],
        ]);
        if ($genotype) {
          $genotype = array_pop($genotype);
          $genotype = (array) $genotype;
        }
        else {
          $genotype = chado_insert_record('genotype', [
            'uniquename' => $call,
            'description' => $call,
            'type_id' => ['name' => 'SNP', 'cv_id' => ['name' => 'sequence']],
          ]);
        }
        $genotype_ids[$call] = $genotype['genotype_id'];
      }

      // For each set requested.
      for ($i = 0; $i < $max; $i++) {

        if ($stocks) {
          $germplasm = $stocks[$i]['germplasm'];
          $sample = $stocks[$i]['sample'];
        }
        else {
          // Create Germplasm.
          $germplasm = factory('chado.stock')->create([
            'organism_id' => $this->organism_id,
          ]);

          // Create Sample.
          $sample = factory('chado.stock')->create([
            'organism_id' => $this->organism_id,
          ]);

          // Link Sample => Germplasm.
          $rel_type = nd_genotypes_get_type_id('Stock Relationship');
          if ($rel_type['position'] == 'subject') {
            $rel = chado_insert_record('stock_relationship', [
              'subject_id' => $sample->stock_id,
              'type_id' => $rel_type['type_id'],
              'object_id' => $germplasm->stock_id,
            ]);
          }
          else {
            $rel = chado_insert_record('stock_relationship', [
              'subject_id' => $germplasm->stock_id,
              'type_id' => $rel_type['type_id'],
              'object_id' => $sample->stock_id,
            ]);
          }
        }

        // For each marker...
        foreach ($this->markers as $marker) {

          // Pick an allele and make sure it exists.
          $call = array_rand($genotype_ids);
          $genotype_id = $genotype_ids[$call];

          $genotype_call = chado_insert_record('genotype_call', [
            'variant_id' => $this->variant_id,
            'marker_id' => $marker->feature_id,
            'genotype_id' => $genotype_id,
            'project_id' => $project->project_id,
            'stock_id' => $sample->stock_id,
          ]);

          $data = [
            'germplasm' => $germplasm,
            'sample' => $sample,
            'marker' => $marker,
            'allele' => $call,
            'genotype_id' => $genotype_id,
            'genotype_call' => $genotype_call,
            'project' => $project,
          ];
          $this->genotypes[] = $data;
        }
      }
      return $this->genotypes;
    }

    /**
     * Save the data in the materialized views.
     */
    public function syncMviews($partition) {
      $return = [];

      $partition = strtolower($partition);

      // Create tables.
      $calls_table = 'mview_ndg_'.$partition.'_calls';
      nd_genotypes_create_mview_ndg_calls($calls_table);
      $variant_table = 'mview_ndg_'.$partition.'_variants';
      nd_genotypes_create_mview_ndg_variants($variant_table);
      $germplasm_table = 'mview_ndg_germplasm_genotyped';
      nd_genotypes_create_mview_ndg_germplasm_genotyped($germplasm_table);

      // Pull out the initial data as well.
      $data = $this->inital_data;

      // Populate the calls table.
      foreach ($this->genotypes as $r) {
        $values = [
          'variant_id' => $this->variant_id,
          'marker_id' => $r['marker']->feature_id,
          'marker_name' => $r['marker']->name,
          'marker_type' => $r['marker']->marker_type,
          'stock_id' => $r['sample']->stock_id,
          'stock_name' => $r['sample']->name,
          'germplasm_id' => $r['germplasm']->stock_id,
          'germplasm_name' => $r['germplasm']->name,
          'project_id' => $r['project']->project_id,
          'genotype_id' => $r['genotype_id'],
          'allele_call' => $r['allele'],
        ];
        $call = chado_insert_record($calls_table, $values);
        $return['calls'][] = (array) $call;

        /* Not sure why this inconsistently fails.
        $germplasm = [
          ':germplasm_id' => $r['germplasm']->stock_id,
          ':germplasm_name' => $r['germplasm']->name,
          ':organism_id' => $data['organism']['organism_id'],
          ':organism_genus' => $data['organism']['genus'],
          ':partition' => $partition,
        ];
        chado_query("
          INSERT INTO {".$germplasm_table."}
            (germplasm_id, germplasm_name,
              organism_id, organism_genus, partition)
            VALUES (:germplasm_id, :germplasm_name,
              :organism_id, :organism_genus, :partition)",
          $germplasm);
          */
      }

      // Finally populate the variant table.
      $values = [
        'variant_id' => $this->variant_id,
        'variant_name' => $data['variant']['record']->name,
        'variant_type' => $data['variant_type']['type'],
        'srcfeature_id' => $data['chromosome']['chromosome_id'],
        'srcfeature_name' => $data['chromosome']['record']->name,
        'fmin' => $data['position']['start'],
        'fmax' => $data['position']['stop'],
        'meta_data' => json_encode(['strand' => 1]),
      ];
      $variant = chado_insert_record($variant_table, $values);
      $return['variant'] = (array) $variant;

      return $return;
    }
}
