#!/usr/bin/env node

/**
 * Script de Avaliação de Desempenho do Banco de Dados
 * 
 * Métricas coletadas:
 * - Tempo médio de inserção (ms)
 * - Tempo médio de consulta diária (ms)
 * - Crescimento estimado do banco (MB/mês)
 * - Taxa de falhas nas gravações (%)
 */

import { PrismaClient } from "@prisma/client";
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const prisma = new PrismaClient();

// Configurações
const TOTAL_INSERTS = parseInt(process.env.TOTAL_INSERTS || '100');
const TOTAL_QUERIES = parseInt(process.env.TOTAL_QUERIES || '50');
const DB_PATH = path.join(__dirname, '../prisma/dev.db');

// Estatísticas
const stats = {
  insertions: {
    total: 0,
    successful: 0,
    failed: 0,
    times: [],
    errors: []
  },
  queries: {
    total: 0,
    successful: 0,
    failed: 0,
    times: [],
    errors: []
  },
  databaseSize: {
    current: 0,
    estimatedGrowthPerMonth: 0
  }
};

/**
 * Obtém o tamanho do arquivo do banco de dados em bytes
 */
async function getDatabaseSize() {
  try {
    const stats = fs.statSync(DB_PATH);
    return stats.size;
  } catch (error) {
    console.error('Erro ao obter tamanho do banco:', error.message);
    return 0;
  }
}

/**
 * Converte bytes para MB
 */
function bytesToMB(bytes) {
  return bytes / (1024 * 1024);
}

/**
 * Obtém ou cria um dispositivo de teste
 */
async function getOrCreateTestDevice() {
  try {
    let device = await prisma.device.findUnique({
      where: { port: 'TEST' }
    });

    if (!device) {
      device = await prisma.device.create({
        data: {
          port: 'TEST',
          name: 'Dispositivo de Teste',
          description: 'Usado para testes de desempenho'
        }
      });
    }

    return device;
  } catch (error) {
    console.error('Erro ao obter/criar dispositivo de teste:', error);
    throw error;
  }
}

/**
 * Testa inserção de Reading
 */
async function testInsertReading(deviceId) {
  const startTime = Date.now();
  
  try {
    await prisma.reading.create({
      data: {
        deviceId: deviceId,
        energyWh: Math.random() * 1000,
        durationMin: Math.floor(Math.random() * 60) + 1
      }
    });
    
    const duration = Date.now() - startTime;
    stats.insertions.successful++;
    stats.insertions.times.push(duration);
    return { success: true, duration };
  } catch (error) {
    const duration = Date.now() - startTime;
    stats.insertions.failed++;
    stats.insertions.errors.push({
      type: 'Reading',
      error: error.message,
      time: Date.now()
    });
    return { success: false, duration, error: error.message };
  }
}

/**
 * Testa inserção de PowerReading
 */
async function testInsertPowerReading(deviceId) {
  const startTime = Date.now();
  
  try {
    await prisma.powerReading.create({
      data: {
        deviceId: deviceId,
        powerW: Math.random() * 500
      }
    });
    
    const duration = Date.now() - startTime;
    stats.insertions.successful++;
    stats.insertions.times.push(duration);
    return { success: true, duration };
  } catch (error) {
    const duration = Date.now() - startTime;
    stats.insertions.failed++;
    stats.insertions.errors.push({
      type: 'PowerReading',
      error: error.message,
      time: Date.now()
    });
    return { success: false, duration, error: error.message };
  }
}

/**
 * Testa consulta diária (leituras do dia)
 */
async function testDailyQuery(deviceId) {
  const startTime = Date.now();
  
  try {
    const today = new Date();
    today.setHours(0, 0, 0, 0);
    
    const readings = await prisma.reading.findMany({
      where: {
        deviceId: deviceId,
        createdAt: {
          gte: today
        }
      },
      orderBy: {
        createdAt: 'desc'
      }
    });
    
    const duration = Date.now() - startTime;
    stats.queries.successful++;
    stats.queries.times.push(duration);
    return { success: true, duration, count: readings.length };
  } catch (error) {
    const duration = Date.now() - startTime;
    stats.queries.failed++;
    stats.queries.errors.push({
      type: 'DailyQuery',
      error: error.message,
      time: Date.now()
    });
    return { success: false, duration, error: error.message };
  }
}

/**
 * Testa consulta de resumo (agregação)
 */
async function testSummaryQuery(deviceId) {
  const startTime = Date.now();
  
  try {
    const readings = await prisma.reading.findMany({
      where: { deviceId: deviceId }
    });

    const totalWh = readings.reduce((sum, r) => sum + r.energyWh, 0);
    const totalKWh = totalWh / 1000;
    
    const duration = Date.now() - startTime;
    stats.queries.successful++;
    stats.queries.times.push(duration);
    return { success: true, duration, totalKWh };
  } catch (error) {
    const duration = Date.now() - startTime;
    stats.queries.failed++;
    stats.queries.errors.push({
      type: 'SummaryQuery',
      error: error.message,
      time: Date.now()
    });
    return { success: false, duration, error: error.message };
  }
}

/**
 * Testa consulta com join (device + readings)
 */
async function testJoinQuery(deviceId) {
  const startTime = Date.now();
  
  try {
    const device = await prisma.device.findUnique({
      where: { id: deviceId },
      include: {
        readings: {
          take: 10,
          orderBy: { createdAt: 'desc' }
        },
        powerReadings: {
          take: 10,
          orderBy: { createdAt: 'desc' }
        }
      }
    });
    
    const duration = Date.now() - startTime;
    stats.queries.successful++;
    stats.queries.times.push(duration);
    return { success: true, duration };
  } catch (error) {
    const duration = Date.now() - startTime;
    stats.queries.failed++;
    stats.queries.errors.push({
      type: 'JoinQuery',
      error: error.message,
      time: Date.now()
    });
    return { success: false, duration, error: error.message };
  }
}

/**
 * Executa testes de inserção
 */
async function runInsertTests(deviceId) {
  console.log(`\n📝 Testando inserções (${TOTAL_INSERTS} operações)...`);
  
  const initialSize = await getDatabaseSize();
  
  for (let i = 0; i < TOTAL_INSERTS; i++) {
    stats.insertions.total++;
    
    // Alterna entre Reading e PowerReading
    if (i % 2 === 0) {
      await testInsertReading(deviceId);
    } else {
      await testInsertPowerReading(deviceId);
    }
    
    // Mostra progresso
    if ((i + 1) % 10 === 0) {
      process.stdout.write(`   Progresso: ${i + 1}/${TOTAL_INSERTS}\r`);
    }
  }
  
  const finalSize = await getDatabaseSize();
  const sizeIncrease = finalSize - initialSize;
  
  console.log(`\n   ✅ Inserções concluídas`);
  console.log(`   📊 Crescimento durante teste: ${bytesToMB(sizeIncrease).toFixed(4)} MB`);
  
  return { initialSize, finalSize, sizeIncrease };
}

/**
 * Executa testes de consulta
 */
async function runQueryTests(deviceId) {
  console.log(`\n🔍 Testando consultas (${TOTAL_QUERIES} operações)...`);
  
  const queryTypes = [
    () => testDailyQuery(deviceId),
    () => testSummaryQuery(deviceId),
    () => testJoinQuery(deviceId)
  ];
  
  for (let i = 0; i < TOTAL_QUERIES; i++) {
    stats.queries.total++;
    
    // Seleciona tipo de consulta aleatoriamente
    const queryType = queryTypes[Math.floor(Math.random() * queryTypes.length)];
    await queryType();
    
    // Mostra progresso
    if ((i + 1) % 10 === 0) {
      process.stdout.write(`   Progresso: ${i + 1}/${TOTAL_QUERIES}\r`);
    }
  }
  
  console.log(`\n   ✅ Consultas concluídas`);
}

/**
 * Calcula estatísticas de inserção
 */
function calculateInsertStats() {
  const times = stats.insertions.times;
  
  if (times.length === 0) {
    return {
      avgTime: 0,
      minTime: 0,
      maxTime: 0,
      p50: 0,
      p95: 0,
      p99: 0
    };
  }
  
  const sorted = [...times].sort((a, b) => a - b);
  const avgTime = times.reduce((a, b) => a + b, 0) / times.length;
  const minTime = sorted[0];
  const maxTime = sorted[sorted.length - 1];
  const p50 = sorted[Math.floor(sorted.length * 0.5)];
  const p95 = sorted[Math.floor(sorted.length * 0.95)];
  const p99 = sorted[Math.floor(sorted.length * 0.99)];
  
  return { avgTime, minTime, maxTime, p50, p95, p99 };
}

/**
 * Calcula estatísticas de consulta
 */
function calculateQueryStats() {
  const times = stats.queries.times;
  
  if (times.length === 0) {
    return {
      avgTime: 0,
      minTime: 0,
      maxTime: 0,
      p50: 0,
      p95: 0,
      p99: 0
    };
  }
  
  const sorted = [...times].sort((a, b) => a - b);
  const avgTime = times.reduce((a, b) => a + b, 0) / times.length;
  const minTime = sorted[0];
  const maxTime = sorted[sorted.length - 1];
  const p50 = sorted[Math.floor(sorted.length * 0.5)];
  const p95 = sorted[Math.floor(sorted.length * 0.95)];
  const p99 = sorted[Math.floor(sorted.length * 0.99)];
  
  return { avgTime, minTime, maxTime, p50, p95, p99 };
}

/**
 * Estima crescimento mensal do banco
 */
function estimateMonthlyGrowth(sizeIncrease, totalInserts) {
  // Assumindo:
  // - Leituras a cada 10 minutos (6 por hora)
  // - PowerReadings a cada 5 minutos (12 por hora)
  // - Total: 18 registros por hora = 432 por dia = 12960 por mês
  
  const insertsPerMonth = 12960; // Estimativa conservadora
  const growthPerInsert = sizeIncrease / totalInserts;
  const estimatedGrowth = growthPerInsert * insertsPerMonth;
  
  return estimatedGrowth;
}

/**
 * Exibe relatório
 */
function displayReport(insertStats, queryStats, sizeInfo) {
  console.log('\n' + '='.repeat(70));
  console.log('📊 RELATÓRIO DE DESEMPENHO DO BANCO DE DADOS');
  console.log('='.repeat(70));
  
  // Tamanho atual do banco
  console.log('\n💾 TAMANHO DO BANCO:');
  console.log(`   Tamanho atual:        ${bytesToMB(sizeInfo.current).toFixed(4)} MB`);
  console.log(`   Crescimento no teste: ${bytesToMB(sizeInfo.sizeIncrease).toFixed(4)} MB`);
  
  // Crescimento estimado
  const estimatedGrowth = estimateMonthlyGrowth(
    sizeInfo.sizeIncrease,
    stats.insertions.successful
  );
  console.log(`   Crescimento estimado:  ${bytesToMB(estimatedGrowth).toFixed(4)} MB/mês`);
  
  // Inserções
  console.log('\n📝 INSERÇÕES:');
  console.log(`   Total:        ${stats.insertions.total}`);
  console.log(`   Sucesso:      ${stats.insertions.successful} (${((stats.insertions.successful / stats.insertions.total) * 100).toFixed(2)}%)`);
  console.log(`   Falhas:       ${stats.insertions.failed} (${((stats.insertions.failed / stats.insertions.total) * 100).toFixed(2)}%)`);
  
  const failureRate = stats.insertions.total > 0
    ? (stats.insertions.failed / stats.insertions.total) * 100
    : 0;
  console.log(`   Taxa de falhas: ${failureRate.toFixed(2)}%`);
  
  console.log('\n⏱️  TEMPO DE INSERÇÃO:');
  console.log(`   Média:        ${insertStats.avgTime.toFixed(2)} ms`);
  console.log(`   Mínimo:       ${insertStats.minTime.toFixed(2)} ms`);
  console.log(`   Máximo:       ${insertStats.maxTime.toFixed(2)} ms`);
  console.log(`   P50:          ${insertStats.p50.toFixed(2)} ms`);
  console.log(`   P95:          ${insertStats.p95.toFixed(2)} ms`);
  console.log(`   P99:          ${insertStats.p99.toFixed(2)} ms`);
  
  // Consultas
  console.log('\n🔍 CONSULTAS:');
  console.log(`   Total:        ${stats.queries.total}`);
  console.log(`   Sucesso:      ${stats.queries.successful} (${((stats.queries.successful / stats.queries.total) * 100).toFixed(2)}%)`);
  console.log(`   Falhas:       ${stats.queries.failed} (${((stats.queries.failed / stats.queries.total) * 100).toFixed(2)}%)`);
  
  console.log('\n⏱️  TEMPO DE CONSULTA DIÁRIA:');
  console.log(`   Média:        ${queryStats.avgTime.toFixed(2)} ms`);
  console.log(`   Mínimo:       ${queryStats.minTime.toFixed(2)} ms`);
  console.log(`   Máximo:       ${queryStats.maxTime.toFixed(2)} ms`);
  console.log(`   P50:          ${queryStats.p50.toFixed(2)} ms`);
  console.log(`   P95:          ${queryStats.p95.toFixed(2)} ms`);
  console.log(`   P99:          ${queryStats.p99.toFixed(2)} ms`);
  
  // Status geral
  console.log('\n✅ STATUS GERAL:');
  if (insertStats.avgTime < 50 && queryStats.avgTime < 100 && failureRate < 1) {
    console.log('   🟢 EXCELENTE - Desempenho ótimo');
  } else if (insertStats.avgTime < 100 && queryStats.avgTime < 200 && failureRate < 5) {
    console.log('   🟡 BOM - Desempenho adequado');
  } else if (insertStats.avgTime < 200 && queryStats.avgTime < 500 && failureRate < 10) {
    console.log('   🟠 ACEITÁVEL - Pode precisar de otimização');
  } else {
    console.log('   🔴 CRÍTICO - Necessita otimização urgente');
  }
  
  // Erros recentes
  if (stats.insertions.errors.length > 0 && stats.insertions.errors.length <= 5) {
    console.log('\n⚠️  ÚLTIMOS ERROS DE INSERÇÃO:');
    stats.insertions.errors.slice(-5).forEach((err, idx) => {
      console.log(`   ${idx + 1}. ${err.type}: ${err.error}`);
    });
  }
  
  if (stats.queries.errors.length > 0 && stats.queries.errors.length <= 5) {
    console.log('\n⚠️  ÚLTIMOS ERROS DE CONSULTA:');
    stats.queries.errors.slice(-5).forEach((err, idx) => {
      console.log(`   ${idx + 1}. ${err.type}: ${err.error}`);
    });
  }
  
  console.log('\n' + '='.repeat(70) + '\n');
}

/**
 * Limpa dados de teste
 */
async function cleanupTestData(deviceId) {
  try {
    console.log('\n🧹 Limpando dados de teste...');
    
    await prisma.reading.deleteMany({
      where: { deviceId: deviceId }
    });
    
    await prisma.powerReading.deleteMany({
      where: { deviceId: deviceId }
    });
    
    console.log('   ✅ Dados de teste removidos');
  } catch (error) {
    console.error('   ⚠️  Erro ao limpar dados de teste:', error.message);
  }
}

/**
 * Função principal
 */
async function main() {
  try {
    console.log('\n🚀 Iniciando Avaliação de Desempenho do Banco de Dados\n');
    console.log(`📊 Configuração:`);
    console.log(`   - Inserções: ${TOTAL_INSERTS}`);
    console.log(`   - Consultas: ${TOTAL_QUERIES}`);
    console.log(`   - Banco: ${DB_PATH}\n`);
    
    // Obtém tamanho inicial
    const initialSize = await getDatabaseSize();
    stats.databaseSize.current = initialSize;
    
    // Obtém ou cria dispositivo de teste
    const testDevice = await getOrCreateTestDevice();
    console.log(`✅ Dispositivo de teste: ${testDevice.name} (ID: ${testDevice.id})\n`);
    
    // Executa testes
    const sizeInfo = await runInsertTests(testDevice.id);
    await runQueryTests(testDevice.id);
    
    // Atualiza tamanho final
    const finalSize = await getDatabaseSize();
    sizeInfo.current = finalSize;
    
    // Calcula estatísticas
    const insertStats = calculateInsertStats();
    const queryStats = calculateQueryStats();
    
    // Exibe relatório
    displayReport(insertStats, queryStats, sizeInfo);
    
    // Salva relatório em JSON
    const reportPath = path.join(__dirname, '../database-performance-report.json');
    const report = {
      timestamp: new Date().toISOString(),
      config: {
        totalInserts: TOTAL_INSERTS,
        totalQueries: TOTAL_QUERIES,
        databasePath: DB_PATH
      },
      database: {
        currentSizeMB: bytesToMB(sizeInfo.current),
        growthDuringTestMB: bytesToMB(sizeInfo.sizeIncrease),
        estimatedGrowthPerMonthMB: bytesToMB(estimateMonthlyGrowth(sizeInfo.sizeIncrease, stats.insertions.successful))
      },
      insertions: {
        total: stats.insertions.total,
        successful: stats.insertions.successful,
        failed: stats.insertions.failed,
        failureRate: stats.insertions.total > 0 ? (stats.insertions.failed / stats.insertions.total) * 100 : 0,
        stats: insertStats
      },
      queries: {
        total: stats.queries.total,
        successful: stats.queries.successful,
        failed: stats.queries.failed,
        stats: queryStats
      },
      errors: {
        insertions: stats.insertions.errors.slice(-10),
        queries: stats.queries.errors.slice(-10)
      }
    };
    
    fs.writeFileSync(reportPath, JSON.stringify(report, null, 2));
    console.log(`💾 Relatório salvo em: ${reportPath}\n`);
    
    // Pergunta se deve limpar dados de teste
    const shouldCleanup = process.env.CLEANUP !== 'false';
    if (shouldCleanup) {
      await cleanupTestData(testDevice.id);
    } else {
      console.log('\n⚠️  Dados de teste mantidos (CLEANUP=false)');
    }
    
  } catch (error) {
    console.error('\n❌ Erro durante o teste:', error);
    process.exit(1);
  } finally {
    await prisma.$disconnect();
  }
}

// Executa
main();

