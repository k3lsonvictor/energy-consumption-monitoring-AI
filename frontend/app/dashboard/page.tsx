'use client';

import { useEffect, useState } from 'react';
import Header from '@/components/Header';
import DeviceCard from '@/components/DeviceCard';
import StatsCard from '@/components/StatsCard';
import { deviceApi, consumoApi, Device, ConsumoData } from '@/lib/api';
import { Zap, DollarSign, Activity, TrendingUp } from 'lucide-react';
import Link from 'next/link';

export default function DashboardPage() {
  const [devices, setDevices] = useState<Device[]>([]);
  const [consumo, setConsumo] = useState<ConsumoData | null>(null);
  const [loading, setLoading] = useState(true);
  const [periodo, setPeriodo] = useState('total');

  useEffect(() => {
    loadData();
  }, [periodo]);

  const loadData = async () => {
    try {
      setLoading(true);
      const [devicesData, consumoData] = await Promise.all([
        deviceApi.getAll(),
        consumoApi.getConsumo(undefined, periodo),
      ]);
      setDevices(devicesData);
      setConsumo(consumoData);
    } catch (error) {
      console.error('Erro ao carregar dados:', error);
    } finally {
      setLoading(false);
    }
  };

  const getDeviceConsumo = (deviceId: number) => {
    if (!consumo) return undefined;
    const deviceConsumo = consumo.dispositivos.find(
      (d) => d.porta === devices.find((dev) => dev.id === deviceId)?.port
    );
    return deviceConsumo
      ? {
          totalKWh: deviceConsumo.totalKWh,
          custoEstimado: deviceConsumo.custoEstimado,
        }
      : undefined;
  };

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

  return (
    <div className="min-h-screen bg-gray-50">
      <Header />
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8">
          <h2 className="text-3xl font-bold text-gray-900 mb-2">Dashboard</h2>
          <p className="text-gray-600">Visão geral dos seus dispositivos e consumo de energia</p>
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
        {consumo && (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 mb-8">
            <StatsCard
              title="Total de Energia"
              value={`${consumo.resumo.totalKWh} kWh`}
              icon={Zap}
              color="primary"
            />
            <StatsCard
              title="Custo Total"
              value={`R$ ${consumo.resumo.custoTotal}`}
              icon={DollarSign}
              color="green"
            />
            <StatsCard
              title="Dispositivos"
              value={devices.length.toString()}
              icon={Activity}
              color="orange"
            />
            <StatsCard
              title="Período"
              value={periodo === 'total' ? 'Total' : periodo === 'hoje' ? 'Hoje' : periodo === 'semana' ? 'Semana' : 'Mês'}
              icon={TrendingUp}
              color="red"
            />
          </div>
        )}

        {/* Lista de dispositivos */}
        <div className="mb-6 flex items-center justify-between">
          <h3 className="text-xl font-semibold text-gray-900">Dispositivos</h3>
          <Link
            href="/devices/manage"
            className="px-4 py-2 bg-primary-600 text-white rounded-lg hover:bg-primary-700 transition-colors"
          >
            Gerenciar Dispositivos
          </Link>
        </div>

        {devices.length === 0 ? (
          <div className="bg-white rounded-lg shadow-md p-12 text-center border border-gray-200">
            <Activity className="w-16 h-16 text-gray-400 mx-auto mb-4" />
            <h3 className="text-lg font-semibold text-gray-900 mb-2">Nenhum dispositivo encontrado</h3>
            <p className="text-gray-600 mb-4">Comece adicionando seu primeiro dispositivo</p>
            <Link
              href="/devices/manage"
              className="inline-block px-6 py-2 bg-primary-600 text-white rounded-lg hover:bg-primary-700 transition-colors"
            >
              Adicionar Dispositivo
            </Link>
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {devices.map((device) => (
              <DeviceCard
                key={device.id}
                device={device}
                consumo={getDeviceConsumo(device.id)}
              />
            ))}
          </div>
        )}
      </main>
    </div>
  );
}

