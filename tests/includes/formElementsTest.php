<?php
namespace Tests\includes;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class formElementsTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests nd_genotypes_element_info().
   *
   * @group form-element
   */
  public function testElementInfo() {
    $elements = nd_genotypes_element_info();
    foreach ($elements as $e) {
      $this->assertArrayHasKey('#input', $e);
      $this->assertArrayHasKey('#process', $e);
      $this->assertArrayHasKey('#theme', $e);
      $this->assertArrayHasKey('#theme_wrappers', $e);
      $this->assertArrayHasKey('#tree', $e);
    }
  }

  /**
   * Tests expand_multiple_organism_stock().
   *
   * @group form-element
   */
  public function testExpandMultipleOrganismStock() {
    $element = [];
    $element['#default_value']['existing'] = [
      0 => ['stock_name' => 'be', 'organism_id' => 1],
      1 => ['stock_name' => 'bo', 'organism_id' => 1],
      2 => ['stock_name' => 'bop', 'organism_id' => 1],
    ];
    $element['#name_autocomplete_path'] = '/fake/path/here';
    $expanded = expand_multiple_organism_stock($element);

    $this->assertArrayHasKey('#default_value', $expanded);
    $this->assertArrayHasKey('#tree', $expanded);
    $this->assertArrayHasKey('#value', $expanded);
    $this->assertArrayHasKey('#organism_title', $expanded);
    $this->assertArrayHasKey('#organism_options', $expanded);
    $this->assertArrayHasKey('#name_title', $expanded);
    $this->assertArrayHasKey('#added_description', $expanded);
    $this->assertArrayHasKey('existing', $expanded);
    $this->assertEquals('be', $expanded['existing'][0]['stock_name']['#default_value']);
    $this->assertEquals('bo', $expanded['existing'][1]['stock_name']['#default_value']);
    $this->assertEquals('bop', $expanded['existing'][2]['stock_name']['#default_value']);
    $this->assertArrayHasKey('new', $expanded);
  }

  /**
   * Tests expand_pairwise_compare_germplasm().
   *
   * @group form-element
   */
  public function testExpandPairwiseCompareGermplasm() {
    $element = [];
    $expanded = expand_pairwise_compare_germplasm($element);

    $this->assertArrayHasKey('#value', $expanded);
    $this->assertArrayHasKey('#options', $expanded);
    $this->assertArrayHasKey('#tree', $expanded);
    $this->assertArrayHasKey('first', $expanded);
    $this->assertEquals('<NONE>', $expanded['first']['#default_value']);
    $this->assertArrayHasKey('second', $expanded);
    $this->assertEquals('<NONE>', $expanded['second']['#default_value']);
  }
}
