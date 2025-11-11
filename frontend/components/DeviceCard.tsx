'use client';

import Link from 'next/link';
import { Device } from '@/lib/api';
import { Zap, Plug, Calendar } from 'lucide-react';

interface DeviceCardProps {
  device: Device;
  consumo?: {
    totalKWh: string;
    custoEstimado: string;
  };
}

export default function DeviceCard({ device, consumo }: DeviceCardProps) {
  return (
    <Link href={`/devices/${device.id}`}>
      <div className="bg-white rounded-lg shadow-md hover:shadow-lg transition-shadow p-6 cursor-pointer border border-gray-200">
        <div className="flex items-start justify-between mb-4">
          <div>
            <h3 className="text-lg font-semibold text-gray-900">{device.name}</h3>
            <p className="text-sm text-gray-500 mt-1">{device.description || 'Sem descrição'}</p>
          </div>
          <div className="flex items-center text-primary-600 bg-primary-50 px-3 py-1 rounded-full">
            <Plug className="w-4 h-4 mr-1" />
            <span className="text-sm font-medium">{device.port}</span>
          </div>
        </div>
        
        {consumo && (
          <div className="mt-4 pt-4 border-t border-gray-200">
            <div className="grid grid-cols-2 gap-4">
              <div>
                <div className="flex items-center text-gray-600 text-sm mb-1">
                  <Zap className="w-4 h-4 mr-1" />
                  Consumo
                </div>
                <p className="text-lg font-semibold text-gray-900">{consumo.totalKWh} kWh</p>
              </div>
              <div>
                <div className="flex items-center text-gray-600 text-sm mb-1">
                  <Calendar className="w-4 h-4 mr-1" />
                  Custo
                </div>
                <p className="text-lg font-semibold text-primary-600">R$ {consumo.custoEstimado}</p>
              </div>
            </div>
          </div>
        )}
      </div>
    </Link>
  );
}

