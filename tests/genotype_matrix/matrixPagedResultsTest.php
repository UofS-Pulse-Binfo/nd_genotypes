<?php
namespace Tests\genotype_matrix;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\GenotypeDatasetSeeder;

class matrixPagedResultsTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test the Genotype Matrix with germplasm but no other filter criteria.
   *
   * @group genotype-matrix
   */
  public function testUnfilteredQuery() {

    // Generate a full dataset.
    $seeder = new GenotypeDatasetSeeder();
    $dataset = $seeder->up();
    $number_of_markers = count($dataset['data']);
    //print_r($data);

    // Germplasm.
    $germplasm_ids = [];
    $germplasm = [];
    foreach ($dataset['germplasm'] as $key => $stock) {
      $germplasm_ids[$key] = $stock->stock_id;
      $germplasm[$key] = [
        'object' => $stock,
        'name' => $stock->name,
        'class' => $stock->name,
      ];
    }

    // Arguements for query.
    $vars = [
      'genus' => 'Tripalus',
      'partition' => 'Tripalus',
      'q' => [],
      'query_args' => [],
      'germplasm' => $germplasm,
      'defaults' => [],
      'header' => [],
      'template_row' => [],
      'variants' => [],
      'data' => [],
    ];
    $vars['q']['germplasm'] = $germplasm_ids;
    $vars['q']['page'] = 1;
    $vars['query_args']['germplasm_id'] = $germplasm_ids;
    $vars['query_args']['page'] = $vars['q']['page'];

    // Now finally, use the function to determine the query.
    $success = nd_genotypes_retrive_matrix_postgresql($vars);
    //var_dump($vars);
    $this->assertNotFalse($success);
    //$this->assertCount($number_of_markers, $vars['variants']);

  }
}
