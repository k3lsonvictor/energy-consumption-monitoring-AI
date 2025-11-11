'use client';

import { useEffect, useState } from 'react';
import Header from '@/components/Header';
import DeviceCard from '@/components/DeviceCard';
import { deviceApi, consumoApi, Device, ConsumoData } from '@/lib/api';
import { Plus, Search } from 'lucide-react';
import Link from 'next/link';

export default function DevicesPage() {
  const [devices, setDevices] = useState<Device[]>([]);
  const [consumo, setConsumo] = useState<ConsumoData | null>(null);
  const [loading, setLoading] = useState(true);
  const [searchTerm, setSearchTerm] = useState('');

  useEffect(() => {
    loadData();
  }, []);

  const loadData = async () => {
    try {
      setLoading(true);
      const [devicesData, consumoData] = await Promise.all([
        deviceApi.getAll(),
        consumoApi.getConsumo(),
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

  const filteredDevices = devices.filter(
    (device) =>
      device.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      device.port.toLowerCase().includes(searchTerm.toLowerCase()) ||
      device.description?.toLowerCase().includes(searchTerm.toLowerCase())
  );

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
        <div className="mb-8 flex items-center justify-between">
          <div>
            <h2 className="text-3xl font-bold text-gray-900 mb-2">Dispositivos</h2>
            <p className="text-gray-600">Visualize todos os seus dispositivos conectados</p>
          </div>
          <Link
            href="/devices/manage"
            className="flex items-center px-4 py-2 bg-primary-600 text-white rounded-lg hover:bg-primary-700 transition-colors"
          >
            <Plus className="w-5 h-5 mr-2" />
            Adicionar Dispositivo
          </Link>
        </div>

        {/* Barra de busca */}
        <div className="mb-6">
          <div className="relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400 w-5 h-5" />
            <input
              type="text"
              placeholder="Buscar dispositivos por nome, porta ou descrição..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              className="w-full pl-10 pr-4 py-3 border border-gray-300 rounded-lg focus:ring-2 focus:ring-primary-500 focus:border-primary-500"
            />
          </div>
        </div>

        {/* Lista de dispositivos */}
        {filteredDevices.length === 0 ? (
          <div className="bg-white rounded-lg shadow-md p-12 text-center border border-gray-200">
            <p className="text-gray-600">
              {searchTerm
                ? 'Nenhum dispositivo encontrado com os critérios de busca'
                : 'Nenhum dispositivo encontrado'}
            </p>
          </div>
        ) : (
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
            {filteredDevices.map((device) => (
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

