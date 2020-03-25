<?php
namespace Tests\includes;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class fieldsTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests nd_genotypes_bundle_fields_info().
   *
   * @group fields
   */
  public function testFieldInfo() {
    $bundle = new \stdClass();
    $entity_type = 'TripalEntity';

    $bundle->data_table = 'stock';
    $fields = nd_genotypes_bundle_fields_info($entity_type, $bundle);
    $this->assertIsArray($fields);
    $this->assertArrayHasKey('local__genotype_matrix_link', $fields);
    $this->assertArrayHasKey('local__germ_marker_summary', $fields);

    $bundle->data_table = 'feature';
    $fields = nd_genotypes_bundle_fields_info($entity_type, $bundle);
    $this->assertIsArray($fields);
    $this->assertArrayHasKey('local__genotype_matrix_link', $fields);
    $this->assertArrayHasKey('local__variant_genotype_summary', $fields);
    $this->assertArrayHasKey('local__marker_genotype_summary', $fields);
    $this->assertArrayHasKey('so__genetic_marker', $fields);
    $this->assertArrayHasKey('local__sequence_with_variants', $fields);

    $this->assertTrue(true);
  }

  /**
   * Tests nd_genotypes_bundle_fields_info().
   *
   * @group fields
   */
  public function testInstancesInfo() {
    $bundle = new \stdClass();
    $entity_type = 'TripalEntity';

    // Stock:
    $bundle->data_table = 'stock';
    $fields = nd_genotypes_bundle_instances_info($entity_type, $bundle);
    $this->assertIsArray($fields);
    $this->assertArrayHasKey('local__germ_marker_summary', $fields);

    $bundle->data_table = 'feature';
    // Sequence Variant:
    $term = tripal_load_term_entity(['vocabulary' => 'SO', 'accession' => '0001060']);
    $bundle->term_id = $term->id;
    $fields = nd_genotypes_bundle_instances_info($entity_type, $bundle);
    $this->assertIsArray($fields);
    $this->assertArrayHasKey('local__variant_genotype_summary', $fields);
    $this->assertArrayHasKey('so__genetic_marker', $fields);
    $this->assertArrayHasKey('local__sequence_with_variants', $fields);
    $this->assertArrayHasKey('local__genotype_matrix_link', $fields);

    // Genetic Marker:
    $term = tripal_load_term_entity(['vocabulary' => 'SO', 'accession' => '0001645']);
    $bundle->term_id = $term->id;
    $fields = nd_genotypes_bundle_instances_info($entity_type, $bundle);
    $this->assertIsArray($fields);
    $this->assertArrayHasKey('local__marker_genotype_summary', $fields);
    $this->assertArrayHasKey('local__sequence_with_variants', $fields);
    $this->assertArrayHasKey('local__genotype_matrix_link', $fields);
  }
}
