# 🗄️ Testes de Desempenho do Banco de Dados

Este script avalia o desempenho do banco de dados SQLite usando Prisma, coletando métricas importantes para monitoramento e otimização.

## 📊 Métricas Coletadas

### 1. Tempo Médio de Inserção (ms)
- Mede o tempo necessário para inserir registros nas tabelas `Reading` e `PowerReading`
- Coleta: média, mínimo, máximo, percentis (P50, P95, P99)

### 2. Tempo Médio de Consulta Diária (ms)
- Testa consultas típicas do sistema:
  - Consultas diárias (leituras do dia atual)
  - Consultas de resumo (agregações)
  - Consultas com JOIN (device + readings)
- Coleta: média, mínimo, máximo, percentis (P50, P95, P99)

### 3. Crescimento Estimado do Banco (MB/mês)
- Calcula o crescimento do banco durante os testes
- Estima crescimento mensal baseado em:
  - Leituras a cada 10 minutos (6 por hora)
  - PowerReadings a cada 5 minutos (12 por hora)
  - Total: ~432 registros por dia = ~12.960 por mês

### 4. Taxa de Falhas nas Gravações (%)
- Monitora erros durante inserções
- Calcula percentual de falhas

## 🚀 Como Usar

### Teste Básico
```bash
npm run test:db
```

### Teste Rápido (menos operações)
```bash
npm run test:db:quick
```

### Teste Completo (mais operações)
```bash
npm run test:db:full
```

### Configuração Customizada
```bash
TOTAL_INSERTS=200 \
TOTAL_QUERIES=100 \
CLEANUP=false \
node scripts/database-performance-test.js
```

### Variáveis de Ambiente

- `TOTAL_INSERTS`: Número de inserções para testar (padrão: `100`)
- `TOTAL_QUERIES`: Número de consultas para testar (padrão: `50`)
- `CLEANUP`: Se deve limpar dados de teste após o teste (padrão: `true`)

## 📈 Interpretando os Resultados

### Tempo de Inserção

- **< 50ms**: 🟢 Excelente
- **50-100ms**: 🟡 Bom
- **100-200ms**: 🟠 Aceitável
- **> 200ms**: 🔴 Necessita otimização

### Tempo de Consulta

- **< 100ms**: 🟢 Excelente
- **100-200ms**: 🟡 Bom
- **200-500ms**: 🟠 Aceitável
- **> 500ms**: 🔴 Necessita otimização

### Taxa de Falhas

- **< 1%**: 🟢 Excelente
- **1-5%**: 🟡 Aceitável
- **5-10%**: 🟠 Preocupante
- **> 10%**: 🔴 Crítico

### Crescimento do Banco

O crescimento estimado ajuda a:
- Planejar espaço em disco
- Decidir sobre estratégias de arquivamento
- Avaliar necessidade de limpeza de dados antigos

## 📝 Exemplo de Saída

```
📊 RELATÓRIO DE DESEMPENHO DO BANCO DE DADOS
======================================================================

💾 TAMANHO DO BANCO:
   Tamanho atual:        0.5234 MB
   Crescimento no teste: 0.0123 MB
   Crescimento estimado:  0.1592 MB/mês

📝 INSERÇÕES:
   Total:        100
   Sucesso:      100 (100.00%)
   Falhas:       0 (0.00%)
   Taxa de falhas: 0.00%

⏱️  TEMPO DE INSERÇÃO:
   Média:        12.45 ms
   Mínimo:       8.23 ms
   Máximo:       45.67 ms
   P50:          11.20 ms
   P95:          28.90 ms
   P99:          42.10 ms

🔍 CONSULTAS:
   Total:        50
   Sucesso:      50 (100.00%)
   Falhas:       0 (0.00%)

⏱️  TEMPO DE CONSULTA DIÁRIA:
   Média:        23.56 ms
   Mínimo:       15.12 ms
   Máximo:       78.34 ms
   P50:          21.45 ms
   P95:          52.30 ms
   P99:          71.20 ms

✅ STATUS GERAL:
   🟢 EXCELENTE - Desempenho ótimo
```

## 🔧 Otimizações Recomendadas

### Se o tempo de inserção estiver alto:

1. **Índices**: Verifique se há índices adequados nas colunas usadas em WHERE
2. **Transações**: Considere usar transações em lote para múltiplas inserções
3. **WAL Mode**: SQLite pode se beneficiar do modo WAL (Write-Ahead Logging)

### Se o tempo de consulta estiver alto:

1. **Índices**: Adicione índices em colunas frequentemente consultadas
   ```sql
   CREATE INDEX idx_reading_device_created ON Reading(deviceId, createdAt);
   CREATE INDEX idx_power_reading_device_created ON PowerReading(deviceId, createdAt);
   ```

2. **Limite de resultados**: Use `take()` para limitar resultados
3. **Seleção de campos**: Selecione apenas campos necessários

### Se a taxa de falhas estiver alta:

1. **Validação**: Verifique se os dados estão sendo validados antes da inserção
2. **Constraints**: Verifique constraints do banco (unique, foreign keys)
3. **Logs**: Analise os erros específicos no relatório JSON

## 📄 Relatório JSON

O script gera um relatório detalhado em JSON: `database-performance-report.json`

```json
{
  "timestamp": "2024-01-15T10:30:00.000Z",
  "config": {
    "totalInserts": 100,
    "totalQueries": 50,
    "databasePath": "./prisma/dev.db"
  },
  "database": {
    "currentSizeMB": 0.5234,
    "growthDuringTestMB": 0.0123,
    "estimatedGrowthPerMonthMB": 0.1592
  },
  "insertions": {
    "total": 100,
    "successful": 100,
    "failed": 0,
    "failureRate": 0.0,
    "stats": {
      "avgTime": 12.45,
      "minTime": 8.23,
      "maxTime": 45.67,
      "p50": 11.20,
      "p95": 28.90,
      "p99": 42.10
    }
  },
  "queries": {
    "total": 50,
    "successful": 50,
    "failed": 0,
    "stats": {
      "avgTime": 23.56,
      "minTime": 15.12,
      "maxTime": 78.34,
      "p50": 21.45,
      "p95": 52.30,
      "p99": 71.20
    }
  }
}
```

## ⚠️ Notas Importantes

1. **Dados de Teste**: Por padrão, o script limpa os dados de teste após a execução. Use `CLEANUP=false` para manter os dados.

2. **Dispositivo de Teste**: O script cria/usa um dispositivo com porta "TEST" para os testes. Este dispositivo não interfere com dados reais.

3. **SQLite**: Este script é otimizado para SQLite. Para PostgreSQL ou MySQL, alguns ajustes podem ser necessários.

4. **Ambiente de Produção**: Execute testes em ambiente de desenvolvimento primeiro. Testes intensivos podem impactar o desempenho.

## 🔍 Troubleshooting

### Erro: "Cannot find module '@prisma/client'"
```bash
cd api
npx prisma generate
```

### Erro: "Database file not found"
- Verifique se o banco existe em `prisma/dev.db`
- Execute `npx prisma migrate dev` se necessário

### Tempos muito altos
- Verifique se há outros processos usando o banco
- Considere executar em ambiente isolado
- Verifique recursos do sistema (CPU, disco)

