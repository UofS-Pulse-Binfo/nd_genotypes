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

    /**
     * Seeds the database with users.
     *
     * @return void
     */
    public function up() {
      $faker = Factory::create();

      // SQL for retrieving the cvterm (assumes sequence cv).
      $get_type_sql = "SELECT cvterm_id FROM {cvterm} WHERE name=:type and cv_id IN (SELECT cv_id FROM {cv} WHERE name='sequence')";

      // Generate a position for this marker.
      $position = rand(1000,100000);
      $this->position;

      // Organism.
      $organism = factory('chado.organism')->create();
      $organism_id = $organism->organism_id;
      $this->organism_id = $organism_id;

      // Chromosome.
      $chr_type_id = chado_query($get_type_sql,
        [':type' => 'chromosome'])->fetchField();
      $chr = factory('chado.feature')->create([
        'organism_id' => $organism->organism_id,
        'type_id' => $chr_type_id,
      ]);
      $chr_id = $chr->feature_id;
      $this->chromosome_id = $chr_id;

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
      $rel_type = variable_get('nd_genotypes_rel_type_id', 0);
      if (!$rel_type) {
        $rel_type = chado_query($get_type_sql,
          [':type' => 'variant_of'])->fetchField();
        variable_set('nd_genotypes_rel_type_id', $rel_type);
      }
      $variant_position = variable_get('nd_genotypes_rel_position', 'subject');
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

      return [
        'marker_id' => $marker_id,
        'record' => $marker,
        'type' => $marker_prop['value'],
      ];

    }
}
