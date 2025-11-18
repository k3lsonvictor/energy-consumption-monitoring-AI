#!/bin/bash

# Script simples de teste de desempenho usando curl
# Uso: ./performance-test-simple.sh [URL] [TOTAL_REQUESTS] [CONCURRENT]

API_URL="${1:-http://localhost:5000}"
TOTAL_REQUESTS="${2:-100}"
CONCURRENT="${3:-10}"

echo "🚀 Teste de Desempenho - API: $API_URL"
echo "📊 Configuração: $TOTAL_REQUESTS requisições, $CONCURRENT concorrentes"
echo ""

# Arquivo temporário para resultados
RESULTS_FILE="/tmp/api_perf_results_$$.txt"
TIMINGS_FILE="/tmp/api_perf_timings_$$.txt"

# Limpa arquivos anteriores
> "$RESULTS_FILE"
> "$TIMINGS_FILE"

# Função para fazer requisição e medir tempo
make_request() {
    local endpoint="$1"
    local start_time=$(date +%s.%N)
    
    response=$(curl -s -w "\n%{http_code}\n%{time_total}" \
        -X GET \
        -H "Content-Type: application/json" \
        "$API_URL$endpoint" \
        2>&1)
    
    local end_time=$(date +%s.%N)
    local duration=$(echo "$end_time - $start_time" | bc)
    
    echo "$response" | tail -2 | head -1 >> "$RESULTS_FILE"  # Status code
    echo "$duration" >> "$TIMINGS_FILE"
    
    # Extrai status code
    local status_code=$(echo "$response" | tail -1)
    echo "$status_code"
}

# Testa endpoint /health
echo "🧪 Testando endpoint: /health"
echo "─────────────────────────────────────────"

success=0
failed=0
error4xx=0
error5xx=0

for i in $(seq 1 $TOTAL_REQUESTS); do
    status=$(make_request "/health")
    
    if [ "$status" -ge 200 ] && [ "$status" -lt 300 ]; then
        ((success++))
    elif [ "$status" -ge 400 ] && [ "$status" -lt 500 ]; then
        ((error4xx++))
        ((failed++))
    elif [ "$status" -ge 500 ]; then
        ((error5xx++))
        ((failed++))
    else
        ((failed++))
    fi
    
    # Mostra progresso a cada 10 requisições
    if [ $((i % 10)) -eq 0 ]; then
        echo -n "."
    fi
done

echo ""
echo ""

# Calcula estatísticas de tempo
if [ -s "$TIMINGS_FILE" ]; then
    # Converte para milissegundos e calcula média
    avg_time=$(awk '{sum+=$1*1000; count++} END {if(count>0) print sum/count; else print 0}' "$TIMINGS_FILE")
    min_time=$(awk '{if(NR==1 || $1*1000<min) min=$1*1000} END {print min}' "$TIMINGS_FILE")
    max_time=$(awk '{if(NR==1 || $1*1000>max) max=$1*1000} END {print max}' "$TIMINGS_FILE")
    
    # Ordena para calcular percentis
    sort -n "$TIMINGS_FILE" > "${TIMINGS_FILE}.sorted"
    total_lines=$(wc -l < "${TIMINGS_FILE}.sorted")
    
    p50_line=$((total_lines * 50 / 100))
    p95_line=$((total_lines * 95 / 100))
    p99_line=$((total_lines * 99 / 100))
    
    p50=$(sed -n "${p50_line}p" "${TIMINGS_FILE}.sorted" | awk '{print $1*1000}')
    p95=$(sed -n "${p95_line}p" "${TIMINGS_FILE}.sorted" | awk '{print $1*1000}')
    p99=$(sed -n "${p99_line}p" "${TIMINGS_FILE}.sorted" | awk '{print $1*1000}')
    
    rm "${TIMINGS_FILE}.sorted"
else
    avg_time=0
    min_time=0
    max_time=0
    p50=0
    p95=0
    p99=0
fi

# Calcula taxa de erro e disponibilidade
error_rate=$(awk "BEGIN {printf \"%.2f\", ($failed / $TOTAL_REQUESTS) * 100}")
availability=$(awk "BEGIN {printf \"%.2f\", ($success / $TOTAL_REQUESTS) * 100}")

# Exibe relatório
echo "============================================================"
echo "📈 RELATÓRIO DE DESEMPENHO"
echo "============================================================"
echo ""
echo "⏱️  TEMPO DE RESPOSTA:"
printf "   Média:        %.2f ms\n" "$avg_time"
printf "   Mínimo:       %.2f ms\n" "$min_time"
printf "   Máximo:       %.2f ms\n" "$max_time"
printf "   P50:          %.2f ms\n" "$p50"
printf "   P95:          %.2f ms\n" "$p95"
printf "   P99:          %.2f ms\n" "$p99"
echo ""
echo "📊 REQUISIÇÕES:"
echo "   Total:        $TOTAL_REQUESTS"
echo "   Sucesso:      $success ($(awk "BEGIN {printf \"%.2f\", ($success / $TOTAL_REQUESTS) * 100}")%)"
echo "   Falhas:       $failed ($(awk "BEGIN {printf \"%.2f\", ($failed / $TOTAL_REQUESTS) * 100}")%)"
echo ""
echo "❌ ERROS HTTP:"
echo "   4xx (Cliente): $error4xx"
echo "   5xx (Servidor): $error5xx"
echo "   Taxa de erro:  $error_rate%"
echo ""
echo "✅ DISPONIBILIDADE:"
printf "   Taxa de sucesso: %.2f%%\n" "$availability"

if (( $(echo "$availability >= 99.9" | bc -l) )); then
    echo "   Status: 🟢 EXCELENTE (99.9%+)"
elif (( $(echo "$availability >= 99.0" | bc -l) )); then
    echo "   Status: 🟡 BOM (99.0%+)"
elif (( $(echo "$availability >= 95.0" | bc -l) )); then
    echo "   Status: 🟠 ACEITÁVEL (95.0%+)"
else
    echo "   Status: 🔴 CRÍTICO (<95.0%)"
fi

echo ""
echo "============================================================"
echo ""

# Limpa arquivos temporários
rm -f "$RESULTS_FILE" "$TIMINGS_FILE"

