![Tripal Dependency](https://img.shields.io/badge/tripal-%3E=3.0-brightgreen)
![Module is Generic](https://img.shields.io/badge/generic-tested%20manually-yellow)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/UofS-Pulse-Binfo/nd_genotypes?include_prereleases)

[![Build Status](https://travis-ci.org/UofS-Pulse-Binfo/nd_genotypes.svg?branch=7.x-3.x)](https://travis-ci.org/UofS-Pulse-Binfo/nd_genotypes)
[![Maintainability](https://api.codeclimate.com/v1/badges/fe04c14638512f7a41f3/maintainability)](https://codeclimate.com/github/UofS-Pulse-Binfo/nd_genotypes/maintainability)
[![Test Coverage](https://api.codeclimate.com/v1/badges/fe04c14638512f7a41f3/test_coverage)](https://codeclimate.com/github/UofS-Pulse-Binfo/nd_genotypes/test_coverage)

# Natural Diversity Genotypes
This module provides support and visualization of genotypic data stored in a modified GMOD Chado schema. The 3.x branch of this module represents a shift towards support for **large scale genotypic datasets** through backwards compatible improvements to the Chado schema including a new gathering table for genotypes (genotype_call) modelled after the chado phenotype table, optimized queries and well-choosen indexes. For benchmarking demonstrating the efficiency of this module, see our [documentation](https://nd-genotypes.readthedocs.io/en/latest/data_storage/benchmarking.html).

## Dependencies
1. Tripal 3.x
2. Tripal Download API
3. PostgreSQL 9.3 (9.4+ recommended)

## Installation
 This module is installed by downloading it and it's dependencies in `[your drupal site]/sites/all/modules` and enabling it through the Drupal Administrative UI.

## Documentation [![Documentation Status](https://readthedocs.org/projects/nd-genotypes/badge/?version=latest)](https://nd-genotypes.readthedocs.io/en/latest/?badge=latest)

Please visit our [online documentation](https://nd-genotypes.readthedocs.io/en/latest/index.html) to learn more about installation, usage and features.
