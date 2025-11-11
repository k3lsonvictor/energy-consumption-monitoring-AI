'use client';

import { useEffect, useState } from 'react';
import { useParams, useRouter } from 'next/navigation';
import Header from '@/components/Header';
import StatsCard from '@/components/StatsCard';
import { deviceApi, consumoApi, Device, Reading, ConsumoData } from '@/lib/api';
import { Zap, DollarSign, Clock, ArrowLeft, TrendingUp } from 'lucide-react';
import Link from 'next/link';
import {
  LineChart,
  Line,
  BarChart,
  Bar,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  Legend,
  ResponsiveContainer,
} from 'recharts';

export default function DeviceDetailPage() {
  const params = useParams();
  const router = useRouter();
  const deviceId = Number(params.id);

  const [device, setDevice] = useState<Device | null>(null);
  const [readings, setReadings] = useState<Reading[]>([]);
  const [consumo, setConsumo] = useState<ConsumoData | null>(null);
  const [loading, setLoading] = useState(true);
  const [periodo, setPeriodo] = useState('total');

  useEffect(() => {
    if (deviceId) {
      loadData();
    }
  }, [deviceId, periodo]);

  const loadData = async () => {
    try {
      setLoading(true);
      const [deviceData, readingsData, consumoData] = await Promise.all([
        deviceApi.getById(deviceId),
        deviceApi.getReadings(deviceId),
        consumoApi.getConsumo(deviceId, periodo),
      ]);
      setDevice(deviceData);
      setReadings(readingsData);
      setConsumo(consumoData);
    } catch (error) {
      console.error('Erro ao carregar dados:', error);
      router.push('/dashboard');
    } finally {
      setLoading(false);
    }
  };

  // Preparar dados para gráficos
  const chartData = readings
    .slice(-20) // Últimas 20 leituras
    .map((reading) => ({
      data: new Date(reading.createdAt).toLocaleDateString('pt-BR'),
      energia: parseFloat(reading.energyWh.toFixed(2)),
      tempo: reading.durationMin,
    }));

  const consumoPorPeriodo = consumo?.dispositivos[0];

  if (loading) {
    return (
      <div className="min-h-screen bg-gray-50">
        <Header />
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
          <div className="flex justify-center items-center h-64">
            <div className="text-gray-500">Carregando...</div>
          </div>
        </div>
      </div>
    );
  }

  if (!device) {
    return null;
  }

  return (
    <div className="min-h-screen bg-gray-50">
      <Header />
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <Link
          href="/dashboard"
          className="inline-flex items-center text-primary-600 hover:text-primary-700 mb-6"
        >
          <ArrowLeft className="w-4 h-4 mr-2" />
          Voltar ao Dashboard
        </Link>

        <div className="mb-8">
          <h2 className="text-3xl font-bold text-gray-900 mb-2">{device.name}</h2>
          <p className="text-gray-600">{device.description || 'Sem descrição'}</p>
          <div className="mt-2">
            <span className="inline-block px-3 py-1 bg-primary-100 text-primary-700 rounded-full text-sm font-medium">
              Porta: {device.port}
            </span>
          </div>
        </div>

        {/* Filtro de período */}
        <div className="mb-6 flex items-center gap-2">
          <span className="text-sm font-medium text-gray-700">Período:</span>
          <select
            value={periodo}
            onChange={(e) => setPeriodo(e.target.value)}
            className="px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-primary-500"
          >
            <option value="hoje">Hoje</option>
            <option value="semana">Última Semana</option>
            <option value="mes">Último Mês</option>
            <option value="total">Total</option>
          </select>
        </div>

        {/* Cards de estatísticas */}
        {consumoPorPeriodo && (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
            <StatsCard
              title="Energia Total"
              value={`${consumoPorPeriodo.totalKWh} kWh`}
              icon={Zap}
              color="primary"
            />
            <StatsCard
              title="Custo Estimado"
              value={`R$ ${consumoPorPeriodo.custoEstimado}`}
              icon={DollarSign}
              color="green"
            />
            <StatsCard
              title="Tempo Total"
              value={`${Math.floor(consumoPorPeriodo.tempoTotalMinutos / 60)}h ${consumoPorPeriodo.tempoTotalMinutos % 60}m`}
              icon={Clock}
              color="orange"
            />
            <StatsCard
              title="Leituras"
              value={consumoPorPeriodo.quantidadeLeituras.toString()}
              icon={TrendingUp}
              color="red"
            />
          </div>
        )}

        {/* Gráficos */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 mb-8">
          {/* Gráfico de linha - Energia ao longo do tempo */}
          <div className="bg-white rounded-lg shadow-md p-6 border border-gray-200">
            <h3 className="text-lg font-semibold text-gray-900 mb-4">Energia Consumida (Wh)</h3>
            <ResponsiveContainer width="100%" height={300}>
              <LineChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="data" />
                <YAxis />
                <Tooltip />
                <Legend />
                <Line
                  type="monotone"
                  dataKey="energia"
                  stroke="#0ea5e9"
                  strokeWidth={2}
                  name="Energia (Wh)"
                />
              </LineChart>
            </ResponsiveContainer>
          </div>

          {/* Gráfico de barras - Duração */}
          <div className="bg-white rounded-lg shadow-md p-6 border border-gray-200">
            <h3 className="text-lg font-semibold text-gray-900 mb-4">Duração das Leituras (min)</h3>
            <ResponsiveContainer width="100%" height={300}>
              <BarChart data={chartData}>
                <CartesianGrid strokeDasharray="3 3" />
                <XAxis dataKey="data" />
                <YAxis />
                <Tooltip />
                <Legend />
                <Bar dataKey="tempo" fill="#10b981" name="Duração (min)" />
              </BarChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* Tabela de leituras recentes */}
        <div className="bg-white rounded-lg shadow-md border border-gray-200">
          <div className="p-6 border-b border-gray-200">
            <h3 className="text-lg font-semibold text-gray-900">Leituras Recentes</h3>
          </div>
          <div className="overflow-x-auto">
            <table className="w-full">
              <thead className="bg-gray-50">
                <tr>
                  <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                    Data/Hora
                  </th>
                  <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                    Energia (Wh)
                  </th>
                  <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                    Duração (min)
                  </th>
                </tr>
              </thead>
              <tbody className="bg-white divide-y divide-gray-200">
                {readings.slice(-10).reverse().map((reading) => (
                  <tr key={reading.id} className="hover:bg-gray-50">
                    <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                      {new Date(reading.createdAt).toLocaleString('pt-BR')}
                    </td>
                    <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                      {reading.energyWh.toFixed(2)}
                    </td>
                    <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                      {reading.durationMin}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </main>
    </div>
  );
}

