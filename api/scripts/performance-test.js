#!/usr/bin/env node

/**
 * Script de Teste de Desempenho da API
 * 
 * Métricas coletadas:
 * - Tempo médio de resposta
 * - Máximo de requisições/segundo
 * - Taxa de erros (HTTP 4xx/5xx)
 * - Disponibilidade estimada
 */

import http from 'http';
import https from 'https';
import { URL } from 'url';

// Configurações
const API_BASE_URL = process.env.API_URL || 'http://localhost:5000';
const CONCURRENT_REQUESTS = parseInt(process.env.CONCURRENT || '10');
const TOTAL_REQUESTS = parseInt(process.env.TOTAL || '1000');
const DURATION_SECONDS = parseInt(process.env.DURATION || '30');
const TEST_MODE = process.env.MODE || 'load'; // 'load' ou 'stress'

// Estatísticas
const stats = {
  totalRequests: 0,
  successfulRequests: 0,
  failedRequests: 0,
  error4xx: 0,
  error5xx: 0,
  responseTimes: [],
  startTime: null,
  endTime: null,
  errors: []
};

// Endpoints para testar
const endpoints = [
  { method: 'GET', path: '/health', name: 'Health Check' },
  { method: 'GET', path: '/devices', name: 'Listar Dispositivos' },
  { method: 'POST', path: '/readings', name: 'Criar Leitura', body: {
    port: '1',
    energyWh: 100.5,
    durationMin: 10,
    realPower: 50.2
  }},
  { method: 'POST', path: '/power', name: 'Criar Leitura de Potência', body: {
    port: '1',
    realPower: 50.2
  }},
  { method: 'GET', path: '/devices/1', name: 'Buscar Dispositivo' }
];

/**
 * Faz uma requisição HTTP
 */
function makeRequest(method, path, body = null) {
  return new Promise((resolve, reject) => {
    const url = new URL(API_BASE_URL + path);
    const options = {
      hostname: url.hostname,
      port: url.port || (url.protocol === 'https:' ? 443 : 80),
      path: url.pathname + url.search,
      method: method,
      headers: {
        'Content-Type': 'application/json',
      }
    };

    if (body) {
      const bodyString = JSON.stringify(body);
      options.headers['Content-Length'] = Buffer.byteLength(bodyString);
    }

    const client = url.protocol === 'https:' ? https : http;
    const startTime = Date.now();

    const req = client.request(options, (res) => {
      let data = '';
      
      res.on('data', (chunk) => {
        data += chunk;
      });

      res.on('end', () => {
        const responseTime = Date.now() - startTime;
        const statusCode = res.statusCode;

        resolve({
          statusCode,
          responseTime,
          data: data.length > 0 ? JSON.parse(data) : null
        });
      });
    });

    req.on('error', (error) => {
      const responseTime = Date.now() - startTime;
      reject({ error, responseTime });
    });

    if (body) {
      req.write(JSON.stringify(body));
    }

    req.setTimeout(10000, () => {
      req.destroy();
      reject({ error: new Error('Request timeout'), responseTime: 10000 });
    });

    req.end();
  });
}

/**
 * Testa um endpoint específico
 */
async function testEndpoint(endpoint) {
  try {
    const result = await makeRequest(endpoint.method, endpoint.path, endpoint.body);
    return result;
  } catch (err) {
    return { error: err.error || err, statusCode: 0, responseTime: err.responseTime || 0 };
  }
}

/**
 * Processa resultado de uma requisição
 */
function processResult(result, endpoint) {
  stats.totalRequests++;
  
  if (result.error) {
    stats.failedRequests++;
    stats.errors.push({
      endpoint: endpoint.name,
      error: result.error.message || 'Unknown error',
      time: Date.now()
    });
    return;
  }

  stats.responseTimes.push(result.responseTime);

  if (result.statusCode >= 200 && result.statusCode < 300) {
    stats.successfulRequests++;
  } else if (result.statusCode >= 400 && result.statusCode < 500) {
    stats.error4xx++;
    stats.failedRequests++;
  } else if (result.statusCode >= 500) {
    stats.error5xx++;
    stats.failedRequests++;
  } else {
    stats.failedRequests++;
  }
}

/**
 * Executa teste de carga
 */
async function runLoadTest() {
  console.log('\n🚀 Iniciando Teste de Desempenho...\n');
  console.log(`📊 Configuração:`);
  console.log(`   - API: ${API_BASE_URL}`);
  console.log(`   - Requisições totais: ${TOTAL_REQUESTS}`);
  console.log(`   - Requisições concorrentes: ${CONCURRENT_REQUESTS}`);
  console.log(`   - Endpoints: ${endpoints.length}\n`);

  stats.startTime = Date.now();
  const endTime = stats.startTime + (DURATION_SECONDS * 1000);

  // Seleciona endpoint aleatório para cada requisição
  const requests = [];
  let requestCount = 0;

  const makeRequests = async () => {
    while (requestCount < TOTAL_REQUESTS && Date.now() < endTime) {
      const endpoint = endpoints[Math.floor(Math.random() * endpoints.length)];
      const promise = testEndpoint(endpoint)
        .then(result => processResult(result, endpoint))
        .catch(err => {
          stats.failedRequests++;
          stats.errors.push({
            endpoint: endpoint.name,
            error: err.message || 'Unknown error',
            time: Date.now()
          });
        });

      requests.push(promise);
      requestCount++;

      // Controla concorrência
      if (requests.length >= CONCURRENT_REQUESTS) {
        await Promise.race(requests);
        requests.splice(requests.findIndex(p => p === promise), 1);
      }

      // Pequeno delay para não sobrecarregar
      await new Promise(resolve => setTimeout(resolve, 10));
    }
  };

  // Aguarda todas as requisições terminarem
  await makeRequests();
  await Promise.all(requests);

  stats.endTime = Date.now();
}

/**
 * Calcula estatísticas
 */
function calculateStats() {
  const duration = (stats.endTime - stats.startTime) / 1000; // em segundos
  
  // Tempo médio de resposta
  const avgResponseTime = stats.responseTimes.length > 0
    ? stats.responseTimes.reduce((a, b) => a + b, 0) / stats.responseTimes.length
    : 0;

  // Tempo mínimo e máximo
  const minResponseTime = stats.responseTimes.length > 0
    ? Math.min(...stats.responseTimes)
    : 0;
  const maxResponseTime = stats.responseTimes.length > 0
    ? Math.max(...stats.responseTimes)
    : 0;

  // Percentis
  const sortedTimes = [...stats.responseTimes].sort((a, b) => a - b);
  const p50 = sortedTimes.length > 0 ? sortedTimes[Math.floor(sortedTimes.length * 0.5)] : 0;
  const p95 = sortedTimes.length > 0 ? sortedTimes[Math.floor(sortedTimes.length * 0.95)] : 0;
  const p99 = sortedTimes.length > 0 ? sortedTimes[Math.floor(sortedTimes.length * 0.99)] : 0;

  // Requisições por segundo
  const requestsPerSecond = duration > 0 ? stats.totalRequests / duration : 0;

  // Taxa de erros
  const errorRate = stats.totalRequests > 0
    ? (stats.failedRequests / stats.totalRequests) * 100
    : 0;

  // Disponibilidade
  const availability = stats.totalRequests > 0
    ? ((stats.successfulRequests / stats.totalRequests) * 100)
    : 0;

  return {
    duration,
    avgResponseTime,
    minResponseTime,
    maxResponseTime,
    p50,
    p95,
    p99,
    requestsPerSecond,
    errorRate,
    availability,
    totalRequests: stats.totalRequests,
    successfulRequests: stats.successfulRequests,
    failedRequests: stats.failedRequests,
    error4xx: stats.error4xx,
    error5xx: stats.error5xx
  };
}

/**
 * Exibe relatório
 */
function displayReport(calculatedStats) {
  console.log('\n' + '='.repeat(60));
  console.log('📈 RELATÓRIO DE DESEMPENHO');
  console.log('='.repeat(60));
  
  console.log('\n⏱️  TEMPO DE RESPOSTA:');
  console.log(`   Média:        ${calculatedStats.avgResponseTime.toFixed(2)} ms`);
  console.log(`   Mínimo:       ${calculatedStats.minResponseTime.toFixed(2)} ms`);
  console.log(`   Máximo:       ${calculatedStats.maxResponseTime.toFixed(2)} ms`);
  console.log(`   P50 (mediana): ${calculatedStats.p50.toFixed(2)} ms`);
  console.log(`   P95:          ${calculatedStats.p95.toFixed(2)} ms`);
  console.log(`   P99:          ${calculatedStats.p99.toFixed(2)} ms`);

  console.log('\n🚀 THROUGHPUT:');
  console.log(`   Requisições/segundo: ${calculatedStats.requestsPerSecond.toFixed(2)} req/s`);
  console.log(`   Duração total:      ${calculatedStats.duration.toFixed(2)} segundos`);

  console.log('\n📊 REQUISIÇÕES:');
  console.log(`   Total:        ${calculatedStats.totalRequests}`);
  console.log(`   Sucesso:      ${calculatedStats.successfulRequests} (${((calculatedStats.successfulRequests / calculatedStats.totalRequests) * 100).toFixed(2)}%)`);
  console.log(`   Falhas:       ${calculatedStats.failedRequests} (${((calculatedStats.failedRequests / calculatedStats.totalRequests) * 100).toFixed(2)}%)`);

  console.log('\n❌ ERROS HTTP:');
  console.log(`   4xx (Cliente): ${calculatedStats.error4xx}`);
  console.log(`   5xx (Servidor): ${calculatedStats.error5xx}`);
  console.log(`   Taxa de erro:  ${calculatedStats.errorRate.toFixed(2)}%`);

  console.log('\n✅ DISPONIBILIDADE:');
  console.log(`   Taxa de sucesso: ${calculatedStats.availability.toFixed(2)}%`);
  
  if (calculatedStats.availability >= 99.9) {
    console.log('   Status: 🟢 EXCELENTE (99.9%+)');
  } else if (calculatedStats.availability >= 99.0) {
    console.log('   Status: 🟡 BOM (99.0%+)');
  } else if (calculatedStats.availability >= 95.0) {
    console.log('   Status: 🟠 ACEITÁVEL (95.0%+)');
  } else {
    console.log('   Status: 🔴 CRÍTICO (<95.0%)');
  }

  // Acessa o objeto global stats.errors (não o parâmetro)
  if (stats.errors && stats.errors.length > 0 && stats.errors.length <= 10) {
    console.log('\n⚠️  ÚLTIMOS ERROS:');
    stats.errors.slice(-10).forEach((err, idx) => {
      console.log(`   ${idx + 1}. ${err.endpoint}: ${err.error}`);
    });
  }

  console.log('\n' + '='.repeat(60) + '\n');
}

/**
 * Função principal
 */
async function main() {
  try {
    await runLoadTest();
    const calculatedStats = calculateStats();
    displayReport(calculatedStats);

    // Salva relatório em JSON
    const fs = await import('fs');
    const reportPath = './performance-report.json';
    fs.writeFileSync(reportPath, JSON.stringify({
      timestamp: new Date().toISOString(),
      config: {
        apiUrl: API_BASE_URL,
        totalRequests: TOTAL_REQUESTS,
        concurrentRequests: CONCURRENT_REQUESTS,
        duration: DURATION_SECONDS
      },
      stats: calculatedStats,
      errors: stats.errors.slice(-50) // Últimos 50 erros
    }, null, 2));
    
    console.log(`💾 Relatório salvo em: ${reportPath}\n`);
  } catch (error) {
    console.error('❌ Erro durante o teste:', error);
    process.exit(1);
  }
}

// Executa
main();

