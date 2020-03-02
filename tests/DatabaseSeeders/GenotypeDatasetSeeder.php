<?php

namespace Tests\DatabaseSeeders;

use StatonLab\TripalTestSuite\Database\Seeder;
use Tests\DatabaseSeeders\GeneticMarkerSeeder;

class GenotypeDatasetSeeder extends Seeder
{
    /**
     * Seeds the database with users.
     *
     * @return void
     */
    public function up() {
      $dataset = [];

      // First grab or create Tripalus databasica.
      $organism = chado_select_record('organism', ['*'], [
        'genus' => 'Tripalus',
        'species' => 'databasica']);
      if ($organism) {
        $organism = array_pop($organism);
      }
      else {
        $organism = factory('chado.organism')->create([
          'genus' => 'Tripalus',
          'species' => 'databasica']);
      }
      $dataset['organism'] = $organism;
      $organism_id = $organism->organism_id;

      // Now create the first genetic marker.
      $seeder = new GeneticMarkerSeeder();
      $data = $seeder->up($organism_id);
      $genotype_data = $seeder->addGenotypes(10);
      $chr_id = $data['chromosome']['chromosome_id'];
      $dataset['data'][0] = $data;
      $dataset['data'][0]['genotypes'] = $genotype_data;
      $dataset['chromosome'] = $data['chromosome'];

      // Extract the germplasm generated for the first set...
      $stocks = [];
      $dataset['germplasm'] = [];
      foreach ($genotype_data as $i => $d) {
        $stocks[$i]['germplasm'] = $d['germplasm'];
        $stocks[$i]['sample'] = $d['sample'];
        $dataset['germplasm'][$i] = $d['germplasm'];
      }

      // Then create an additional 99 markers.
      for ($i=1; $i <= 100; $i++) {
        $seeder = new GeneticMarkerSeeder();
        $data = $seeder->up($organism_id, $chr_id);
        $genotype_data = $seeder->addGenotypes(10, $stocks);

        $dataset['data'][$i] = $data;
        $dataset['data'][$i]['genotypes'] = $genotype_data;
      }

      return $dataset;
    }
}
