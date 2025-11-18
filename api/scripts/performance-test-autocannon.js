#!/usr/bin/env node

/**
 * Script de Teste de Desempenho usando Autocannon
 * 
 * Instalação: npm install -g autocannon
 * Uso: node scripts/performance-test-autocannon.js
 */

import { exec } from 'child_process';
import { promisify } from 'util';

const execAsync = promisify(exec);

const API_BASE_URL = process.env.API_URL || 'http://localhost:5000';
const DURATION = process.env.DURATION || '30'; // segundos
const CONNECTIONS = process.env.CONNECTIONS || '10';
const PIPELINING = process.env.PIPELINING || '1';

// Endpoints para testar
const endpoints = [
  { path: '/health', name: 'Health Check' },
  { path: '/devices', name: 'Listar Dispositivos' }
];

/**
 * Executa teste com autocannon
 */
async function runAutocannonTest(endpoint) {
  console.log(`\n🧪 Testando: ${endpoint.name} (${endpoint.path})\n`);

  const command = `autocannon -c ${CONNECTIONS} -p ${PIPELINING} -d ${DURATION} --json ${API_BASE_URL}${endpoint.path}`;

  try {
    const { stdout, stderr } = await execAsync(command);
    
    if (stderr) {
      console.error('Erro:', stderr);
    }

    const result = JSON.parse(stdout);
    return {
      endpoint: endpoint.name,
      path: endpoint.path,
      ...result
    };
  } catch (error) {
    console.error(`Erro ao testar ${endpoint.name}:`, error.message);
    return null;
  }
}

/**
 * Formata e exibe resultados
 */
function displayResults(results) {
  console.log('\n' + '='.repeat(70));
  console.log('📊 RELATÓRIO DE DESEMPENHO - AUTOCANNON');
  console.log('='.repeat(70));

  results.forEach(result => {
    if (!result) return;

    console.log(`\n📌 ${result.endpoint} (${result.path})`);
    console.log('─'.repeat(70));
    
    const latency = result.latency;
    const requests = result.requests;
    const throughput = result.throughput;

    console.log('\n⏱️  LATÊNCIA:');
    console.log(`   Média:    ${latency.mean.toFixed(2)} ms`);
    console.log(`   Mínimo:   ${latency.min.toFixed(2)} ms`);
    console.log(`   Máximo:   ${latency.max.toFixed(2)} ms`);
    console.log(`   P50:      ${latency.p50.toFixed(2)} ms`);
    console.log(`   P90:      ${latency.p90.toFixed(2)} ms`);
    console.log(`   P99:      ${latency.p99.toFixed(2)} ms`);
    console.log(`   P99.9:    ${latency.p99_9.toFixed(2)} ms`);

    console.log('\n🚀 THROUGHPUT:');
    console.log(`   Requisições/segundo: ${requests.mean.toFixed(2)} req/s`);
    console.log(`   Total de requisições: ${requests.total}`);
    console.log(`   Bytes/segundo:        ${(throughput.mean / 1024).toFixed(2)} KB/s`);

    console.log('\n📊 STATUS:');
    const statusCodes = result.statusCodeStats || {};
    Object.keys(statusCodes).forEach(code => {
      const count = statusCodes[code];
      const percentage = (count / requests.total * 100).toFixed(2);
      console.log(`   ${code}: ${count} (${percentage}%)`);
    });

    // Calcular disponibilidade
    const successCodes = Object.keys(statusCodes)
      .filter(code => code >= 200 && code < 300)
      .reduce((sum, code) => sum + statusCodes[code], 0);
    const availability = (successCodes / requests.total * 100).toFixed(2);
    
    console.log('\n✅ DISPONIBILIDADE:');
    console.log(`   Taxa de sucesso: ${availability}%`);
    
    if (availability >= 99.9) {
      console.log('   Status: 🟢 EXCELENTE');
    } else if (availability >= 99.0) {
      console.log('   Status: 🟡 BOM');
    } else if (availability >= 95.0) {
      console.log('   Status: 🟠 ACEITÁVEL');
    } else {
      console.log('   Status: 🔴 CRÍTICO');
    }
  });

  console.log('\n' + '='.repeat(70) + '\n');
}

/**
 * Função principal
 */
async function main() {
  console.log('🚀 Iniciando Teste de Desempenho com Autocannon\n');
  console.log(`📊 Configuração:`);
  console.log(`   - API: ${API_BASE_URL}`);
  console.log(`   - Duração: ${DURATION}s`);
  console.log(`   - Conexões: ${CONNECTIONS}`);
  console.log(`   - Pipeline: ${PIPELINING}`);

  // Verifica se autocannon está instalado
  try {
    await execAsync('which autocannon');
  } catch (error) {
    console.error('\n❌ Autocannon não encontrado!');
    console.log('\n📦 Para instalar:');
    console.log('   npm install -g autocannon');
    console.log('\n   Ou use o script alternativo:');
    console.log('   node scripts/performance-test.js\n');
    process.exit(1);
  }

  const results = [];
  for (const endpoint of endpoints) {
    const result = await runAutocannonTest(endpoint);
    if (result) {
      results.push(result);
    }
  }

  displayResults(results);

  // Salva relatório
  const fs = await import('fs');
  const reportPath = './performance-report-autocannon.json';
  fs.writeFileSync(reportPath, JSON.stringify({
    timestamp: new Date().toISOString(),
    config: {
      apiUrl: API_BASE_URL,
      duration: DURATION,
      connections: CONNECTIONS,
      pipelining: PIPELINING
    },
    results
  }, null, 2));

  console.log(`💾 Relatório salvo em: ${reportPath}\n`);
}

main().catch(console.error);

