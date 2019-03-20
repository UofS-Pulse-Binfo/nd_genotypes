<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

/**
 * Tests the following API functions:
 *   - nd_genotypes_get_IUPAC_code()
 *   - nd_genotypes_expand_IUPAC_code()
 *   - nd_genotype_get_consensus_call()
 *   - nd_genotypes_complement_calls()
 *   - nd_genotypes_get_alleles_for_variant()
 */
class allelesTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests nd_genotypes_get_IUPAC_code().
   */
  public function testGetIUPAC() {

    $test_combinations = [
      'CG' => 'S',
      'AT' => 'W',
      'CGT' => 'B',
      'C' => 'C',
      'CGGCCGCCG' => 'S',
      'TTTTTTTTT' => 'T',
    ];
    foreach ($test_combinations as $alleles => $code) {
      $result = nd_genotypes_get_IUPAC_code($alleles);
      $this->assertEquals($code, $result);
    }
  }
 
  /**
   * Tests nd_genotypes_expand_IUPAC_code().
   */
  public function testExpandIUPAC() {

    $test_combintations = [
      'S' => 'CG',
      'W' => 'AT',
      'B' => 'CGT',
      'C' => 'C',
      'CTC' => 'CT',
      'TTTTTTTT' => 'T',
      'SB' => 'CGT',
    ];

    foreach ($test_combintations as $codes => $alleles) {
      $result = nd_genotypes_expand_IUPAC_code($codes);
      $this->assertEquals($alleles, $result);
    }
  }

  /**
   * Tests nd_genotype_get_consensus_call().
   */
  public function testGetConsensusCall() {

    $test_combinations = [
      'A' => ['A','A','A','A','A','A','T'],
      'T' => ['T'],
      'C' => ['A','C','A','C','A','C','C'],
      NULL => [],
    ];
    foreach ($test_combinations as $consensus => $alleles) {
      $result = nd_genotype_get_consensus_call($alleles);
      $this->assertEquals($consensus, $result);
    }
  }

  /**
   * Tests nd_genotypes_complement_calls().
   */
  public function testComplementCalls() {

    $calls = 'T';
    $expect = 'A';
    $result = nd_genotypes_complement_calls($calls);
    $this->assertEquals($expect, $result);

    $calls = ['T'];
    $expect = ['A'];
    $result = nd_genotypes_complement_calls($calls);
    $this->assertEquals($expect, array_values($result));

    $calls = ['T', 'T', 'C', 'A'];
    $expect = ['A', 'G', 'T'];
    $result = nd_genotypes_complement_calls($calls);
    $this->assertEquals($expect, array_values($result));

  }

  /**
   * Tests nd_genotypes_get_alleles_for_variant().
   *
   * @todo
   *
  public function testGetAlleles4Variant() {
    $this->assertTrue(true);
  }*/
}
